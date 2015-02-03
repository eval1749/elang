// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/cg/code_generator.h"

#include "base/logging.h"
#include "elang/base/temporary_change_value.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/cg/type_mapper.h"
#include "elang/compiler/cg/variable_analyzer.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/editor.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/intrinsic_names.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::Output
//
struct CodeGenerator::Output final {
  hir::Value* value;

  Output() : value(nullptr) {}
};

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::ScopedOutput
// To set output of current context for visitor.
//
class CodeGenerator::ScopedOutput final {
 public:
  explicit ScopedOutput(CodeGenerator* generator);
  ~ScopedOutput();

 private:
  CodeGenerator* const generator_;
  Output output_;
  Output* const previous_output_;

  DISALLOW_COPY_AND_ASSIGN(ScopedOutput);
};

CodeGenerator::ScopedOutput::ScopedOutput(CodeGenerator* generator)
    : generator_(generator), previous_output_(generator->output_) {
  generator_->output_ = &output_;
}

CodeGenerator::ScopedOutput::~ScopedOutput() {
  DCHECK_EQ(generator_->output_, &output_);
  generator_->output_ = previous_output_;
}

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::BreakContext represents target blocks of |break| and
// |continue| statements. |switch| statement also specify |continue_block|
// from outer |BreakContext|.
//
struct CodeGenerator::BreakContext final {
  hir::BasicBlock* break_block;
  hir::BasicBlock* continue_block;
  const BreakContext* outer;

  BreakContext(const BreakContext* outer,
               hir::BasicBlock* break_block,
               hir::BasicBlock* continue_block)
      : break_block(break_block),
        continue_block(continue_block),
        outer(outer) {}
};

class CodeGenerator::ScopedBreakContext final {
 public:
  ScopedBreakContext(CodeGenerator* generator,
                     hir::BasicBlock* break_block,
                     hir::BasicBlock* continue_block);
  ~ScopedBreakContext();

 private:
  CodeGenerator* const generator_;
  BreakContext const context_;

  DISALLOW_COPY_AND_ASSIGN(ScopedBreakContext);
};

CodeGenerator::ScopedBreakContext::ScopedBreakContext(
    CodeGenerator* generator,
    hir::BasicBlock* break_block,
    hir::BasicBlock* continue_block)
    : generator_(generator),
      context_(generator_->break_context_, break_block, continue_block) {
  generator_->break_context_ = &context_;
}

CodeGenerator::ScopedBreakContext::~ScopedBreakContext() {
  DCHECK_EQ(generator_->break_context_, &context_);
  generator_->break_context_ = context_.outer;
}

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
CodeGenerator::CodeGenerator(CompilationSession* session,
                             hir::Factory* factory,
                             VariableAnalyzer* variable_analyzer)
    : CompilationSessionUser(session),
      break_context_(nullptr),
      editor_(nullptr),
      factory_(factory),
      function_(nullptr),
      output_(nullptr),
      type_mapper_(new TypeMapper(session, factory)),
      variable_analyzer_(variable_analyzer),
      void_type_(MapType(PredefinedName::Void)) {
}

CodeGenerator::~CodeGenerator() {
}

hir::Type* CodeGenerator::bool_type() const {
  return MapType(PredefinedName::Bool);
}

hir::TypeFactory* CodeGenerator::types() const {
  return factory()->types();
}

hir::Value* CodeGenerator::void_value() const {
  return void_type()->default_value();
}

void CodeGenerator::Commit() {
  auto const is_valid = editor()->Commit();
  DCHECK(is_valid) << editor()->errors();
}

void CodeGenerator::Emit(hir::Instruction* instruction) {
  editor()->Append(instruction);
}

hir::BasicBlock* CodeGenerator::EmitMergeBlock() {
  auto const block = editor()->basic_block();
  Commit();
  return editor()->SplitBefore(block->last_instruction());
}

void CodeGenerator::EmitOutput(hir::Value* value) {
  DCHECK(value);
  DCHECK_NE(value, void_value());
  if (!output_)
    return;
  DCHECK(!output_->value);
  output_->value = value;
}

void CodeGenerator::EmitOutputInstruction(hir::Instruction* instruction) {
  Emit(instruction);
  EmitOutput(static_cast<hir::Value*>(instruction));
}

void CodeGenerator::EmitParameterBindings(ast::Method* ast_method) {
  if (ast_method->parameters().empty())
    return;
  auto const entry = function_->entry_block()->first_instruction();
  if (ast_method->parameters().size() == 1u) {
    // Take parameter from `entry` instruction.
    EmitVariableBinding(ast_method->parameters()[0], nullptr, entry);
    return;
  }
  auto index = 0;
  for (auto const parameter : ast_method->parameters()) {
    auto const get_instr = factory()->NewGetInstruction(entry, index);
    Emit(get_instr);
    EmitVariableBinding(parameter, nullptr, get_instr);
    ++index;
  }
}

void CodeGenerator::EmitVariableAssignment(ast::NamedNode* ast_node,
                                           ast::Expression* ast_value) {
  auto const variable = ValueOf(ast_node)->as<ir::Variable>();
  auto const value = GenerateValue(ast_value);
  auto const home = variables_[variable]->as<hir::Instruction>();
  DCHECK(home);
  variable_analyzer_->DidSetVariable(home, editor()->basic_block());
  Emit(factory()->NewStoreInstruction(home, value));
  EmitOutput(value);
}

// |ast_value| comes from `var` statement
// |value| comes from parameter
void CodeGenerator::EmitVariableBinding(ast::NamedNode* ast_variable,
                                        ast::Expression* ast_value,
                                        hir::Value* value) {
  auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
  auto const variable_type = MapType(variable->type());
  auto const variable_value =
      value ? value : ast_value ? GenerateValue(ast_value)
                                : variable_type->default_value();

  if (variable->storage() == ir::StorageClass::Void)
    return;

  if (variable->storage() == ir::StorageClass::ReadOnly) {
    DCHECK(!variables_.count(variable));
    variables_[variable] = variable_value;
    return;
  }

  DCHECK_EQ(variable->storage(), ir::StorageClass::Local);
  auto const alloc_instr =
      factory()->NewStackAllocInstruction(variable_type, 1);
  DCHECK(!variables_.count(variable));
  variables_[variable] = alloc_instr;
  Emit(alloc_instr);
  Emit(factory()->NewStoreInstruction(alloc_instr, variable_value));
  variable_analyzer_->RegisterVariable(alloc_instr);
}

void CodeGenerator::EmitVariableReference(ast::NamedNode* ast_variable) {
  if (!NeedOutput())
    return;
  auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
  DCHECK(variable);
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end()) << *variable << " isn't resolved";
  DCHECK(it->second) << *variable << " has no value";
  if (variable->storage() == ir::StorageClass::ReadOnly) {
    EmitOutput(it->second);
    return;
  }
  auto const home = it->second->as<hir::Instruction>();
  DCHECK(home);
  variable_analyzer_->DidUseVariable(home, editor()->basic_block());
  EmitOutputInstruction(factory()->NewLoadInstruction(home));
}

hir::Function* CodeGenerator::FunctionOf(ast::Method* ast_method) const {
  auto const it = functions_.find(ast_method);
  return it == functions_.end() ? nullptr : it->second;
}

void CodeGenerator::Generate(ast::Statement* statement) {
  DCHECK(!output_);
  if (!statement)
    return;
  statement->Accept(this);
}

// Generate:
//  T* %ptr = element %array, %index
// Or:
//  {int,int} %indexes = tuple %index0, %index1
//  T* %ptr = element %array, %indexes
hir::Value* CodeGenerator::GenerateArrayAccess(ast::ArrayAccess* node) {
  // TODO(eval1749) NYI: array bounds check.
  auto const array = GenerateValue(node->array());
  std::vector<hir::Value*> index_values(node->indexes().size());
  index_values.resize(0);
  std::vector<hir::Type*> index_types(node->indexes().size());
  index_types.resize(0);
  for (auto index : node->indexes()) {
    index_values.push_back(GenerateValue(index));
    index_types.push_back(index_values.back()->type());
  }
  DCHECK(!index_values.empty());
  DCHECK_EQ(index_values.size(), index_types.size());
  if (index_values.size() == 1u) {
    auto const element_instr =
        factory()->NewElementInstruction(array, index_values.front());
    Emit(element_instr);
    return element_instr;
  }
  auto const indexes_type = types()->NewTupleType(index_types);
  auto const indexes_instr =
      factory()->NewTupleInstruction(indexes_type, index_values);
  Emit(indexes_instr);
  auto const element_instr =
      factory()->NewElementInstruction(array, indexes_instr);
  Emit(element_instr);
  return element_instr;
}

hir::Value* CodeGenerator::GenerateBool(ast::Expression* expression) {
  // TOOD(eval1749) Convert |condition| to |bool|
  auto const value = GenerateValue(expression);
  DCHECK_EQ(value->type(), bool_type());
  return value;
}

void CodeGenerator::GenerateDoOrWhile(ast::DoOrWhileStatement* node) {
  auto const for_statement = node->as<ast::ForStatement>();
  DCHECK(!for_statement || !for_statement->step());

  auto const head_block = editor()->basic_block();
  Commit();

  auto const break_block =
      editor()->SplitBefore(head_block->last_instruction());

  auto const while_block = editor()->NewBasicBlock(break_block);
  auto const continue_block = editor()->NewBasicBlock(while_block);

  editor()->Continue(head_block);
  editor()->SetBranch(node->token() == TokenType::Do ? continue_block
                                                     : while_block);
  if (for_statement)
    Generate(for_statement->initializer());
  Commit();

  editor()->Edit(continue_block);
  editor()->SetBranch(while_block);
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    Generate(node->statement());
  }
  Commit();

  editor()->Edit(while_block);
  editor()->SetBranch(GenerateBool(node->condition()), continue_block,
                      break_block);
  Commit();
  editor()->Edit(break_block);
}

hir::Value* CodeGenerator::GenerateValue(ast::Expression* expression) {
  ScopedOutput output(this);
  expression->Accept(this);
  DCHECK(output_->value);
  return output_->value;
}

hir::Value* CodeGenerator::GenerateValueAs(ast::Expression* expression,
                                           hir::Type* type) {
  auto const value = GenerateValue(expression);
  if (value->type() == type)
    return value;
  auto const instr = factory()->NewStaticCastInstruction(type, value);
  Emit(instr);
  return instr;
}

hir::Type* CodeGenerator::MapType(PredefinedName name) const {
  return type_mapper_->Map(name);
}

hir::Type* CodeGenerator::MapType(ir::Type* type) const {
  return type_mapper_->Map(type);
}

bool CodeGenerator::NeedOutput() const {
  return !!output_;
}

hir::Instruction* CodeGenerator::NewInstructionFor(ast::Expression* node,
                                                   hir::Type* type,
                                                   hir::Value* left,
                                                   hir::Value* right) {
  switch (node->op()->type()) {
#define V(Name, ...)    \
  case TokenType::Name: \
    return factory()->New##Name##Instruction(type, left, right);
    FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
    FOR_EACH_BITWISE_BINARY_OPERATION(V)
    FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V
#define V(Name, ...)    \
  case TokenType::Name: \
    return factory()->New##Name##Instruction(left, right);
    FOR_EACH_EQUALITY_OPERATION(V)
    FOR_EACH_RELATIONAL_OPERATION(V)
#undef V
    default:
      NOTREACHED() << "Unsupported operator: " << *node;
      return nullptr;
  }
}

hir::Value* CodeGenerator::NewLiteral(hir::Type* type, const Token* token) {
  if (type == MapType(PredefinedName::Bool))
    return factory()->NewBoolLiteral(token->bool_data());

  if (type == MapType(PredefinedName::Char))
    return factory()->NewCharLiteral(token->char_data());

  if (type == MapType(PredefinedName::Float32))
    return factory()->NewFloat32Literal(token->f32_data());

  if (type == MapType(PredefinedName::Float64))
    return factory()->NewFloat64Literal(token->f64_data());

  if (type == MapType(PredefinedName::Int16)) {
    return factory()->NewInt16Literal(
        static_cast<int16_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::Int32)) {
    return factory()->NewInt32Literal(
        static_cast<int32_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::Int64))
    return factory()->NewInt64Literal(token->int64_data());

  if (type == MapType(PredefinedName::Int8)) {
    return factory()->NewInt8Literal(static_cast<int8_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::UInt16)) {
    return factory()->NewUInt16Literal(
        static_cast<uint16_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::UInt32)) {
    return factory()->NewUInt32Literal(
        static_cast<uint32_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::UInt64)) {
    return factory()->NewUInt64Literal(
        static_cast<uint64_t>(token->int64_data()));
  }

  if (type == MapType(PredefinedName::UInt8)) {
    return factory()->NewUInt8Literal(
        static_cast<uint8_t>(token->int64_data()));
  }

  NOTREACHED() << "Bad literal token " << *token;
  return nullptr;
}

// The entry point of |CodeGenerator|.
bool CodeGenerator::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

ir::Node* CodeGenerator::ValueOf(ast::Node* node) const {
  return semantics()->ValueOf(node);
}

//
// ast::Visitor declaration nodes
//

void CodeGenerator::VisitMethod(ast::Method* ast_method) {
  DCHECK(!editor_);
  DCHECK(!function_);
  //  1 Convert ast::FunctionType to hir::FunctionType
  //  2 hir::NewFunction(function_type)
  auto const method = ValueOf(ast_method)->as<ir::Method>();
  if (!method) {
    DVLOG(0) << "Not resolved " << *ast_method;
    return;
  }
  auto const ast_method_body = ast_method->body();
  if (!ast_method_body)
    return;
  auto const function = factory()->NewFunction(
      type_mapper()->Map(method->signature())->as<hir::FunctionType>());
  TemporaryChangeValue<hir::Function*> function_scope(function_, function);
  functions_[ast_method] = function;
  variable_analyzer_->RegisterFunction(function);

  hir::Editor editor(factory(), function_);
  TemporaryChangeValue<hir::Editor*> editor_scope(editor_, &editor);
  editor.Edit(function->entry_block());

  EmitParameterBindings(ast_method);

  if (auto const ast_expression = ast_method_body->as<ast::Expression>()) {
    editor.SetReturn(GenerateValue(ast_expression));
    return;
  }
  Generate(ast_method_body);
  if (!editor.basic_block())
    return;
  if (MapType(method->return_type()) != MapType(PredefinedName::Void) &&
      (editor.basic_block() == function->entry_block() ||
       editor.basic_block()->HasPredecessor())) {
    Error(ErrorCode::CodeGeneratorReturnNone, ast_method);
  }
  editor.Commit();
}

//
// ast::Visitor expression nodes
//

void CodeGenerator::VisitArrayAccess(ast::ArrayAccess* node) {
  auto const element_instr = GenerateArrayAccess(node);
  EmitOutputInstruction(factory()->NewLoadInstruction(element_instr));
}

// There are five patterns:
//  1. parameter = expression
//  2. variable = expression
//  3. array[index+] = expression
//  5. name = expression; field or property assignment
//  4. container.member = expression; member assignment
void CodeGenerator::VisitAssignment(ast::Assignment* node) {
  auto const lhs = node->left();
  auto const rhs = node->right();
  if (auto const reference = lhs->as<ast::ParameterReference>()) {
    EmitVariableAssignment(reference->parameter(), rhs);
    return;
  }
  if (auto const reference = lhs->as<ast::VariableReference>()) {
    EmitVariableAssignment(reference->variable(), rhs);
    return;
  }
  if (auto const reference = lhs->as<ast::ArrayAccess>()) {
    auto const pointer = GenerateArrayAccess(reference);
    auto const value = GenerateValue(rhs);
    EmitOutputInstruction(factory()->NewStoreInstruction(pointer, value));
    return;
  }
  if (auto const reference = lhs->as<ast::NameReference>()) {
    DVLOG(0) << "NYI Assign to field " << *lhs;
    return;
  }
  if (auto const reference = lhs->as<ast::MemberAccess>()) {
    DVLOG(0) << "NYI Assign to field " << *lhs;
    return;
  }
  NOTREACHED() << "Invalid left value " << *lhs;
}

void CodeGenerator::VisitBinaryOperation(ast::BinaryOperation* node) {
  DCHECK(NeedOutput());
  if (node->is_conditional()) {
    // Generate "&&" or "||"
    //  "&&"                        "||"
    //  left:
    //   ... left ...
    //   br %left, right, merge     br %left, merge, right
    //  right:
    //   ... right ...
    //   br merge
    //  merge:
    //   bool %out = phi left: %left, right: %right
    auto const left_value = GenerateBool(node->left());
    auto const left_block = editor()->basic_block();
    auto const merge_block = EmitMergeBlock();
    auto const right_block = editor()->EditNewBasicBlock(merge_block);
    auto const right_value = GenerateBool(node->right());
    editor()->SetBranch(merge_block);
    Commit();
    editor()->Continue(left_block);
    if (node->op() == TokenType::And)
      editor()->SetBranch(left_value, right_block, merge_block);
    else
      editor()->SetBranch(left_value, merge_block, right_block);
    Commit();
    editor()->Edit(merge_block);
    auto const phi = editor()->NewPhi(bool_type());
    editor()->SetPhiInput(phi, left_block, left_value);
    editor()->SetPhiInput(phi, right_block, right_value);
    EmitOutput(phi);
    return;
  }

  auto const ir_type = ValueOf(node)->as<ir::Class>();
  DCHECK(ir_type) << "NYI user defined operator: " << *node;
  auto const type = MapType(ir_type);
  auto const left = GenerateValueAs(node->left(), type);
  auto const right = GenerateValueAs(node->right(), type);
  auto const instr = NewInstructionFor(node, type, left, right);
  EmitOutputInstruction(instr);
}

// Generate function call by generating callee, arguments from left to right.
void CodeGenerator::VisitCall(ast::Call* node) {
  auto const ir_callee = ValueOf(node->callee())->as<ir::Method>();
  DCHECK(ir_callee) << "Unresolved call" << *node;
  auto const callee = GenerateValue(node->callee());
  if (node->arguments().empty()) {
    EmitOutputInstruction(factory()->NewCallInstruction(callee, void_value()));
    return;
  }
  if (node->arguments().size() == 1u) {
    auto const argument = GenerateValue(node->arguments().front());
    EmitOutputInstruction(factory()->NewCallInstruction(callee, argument));
    return;
  }
  // Generate argument list.
  std::vector<hir::Value*> arguments(node->arguments().size());
  arguments.resize(0);
  for (auto const argument : node->arguments())
    arguments.push_back(GenerateValue(argument));
  auto const args_instr = factory()->NewTupleInstruction(
      callee->type()->as<hir::FunctionType>()->parameters_type(), arguments);
  Emit(args_instr);
  EmitOutputInstruction(factory()->NewCallInstruction(callee, args_instr));
}

void CodeGenerator::VisitConditional(ast::Conditional* node) {
  auto const cond_value = GenerateBool(node->condition());
  auto const cond_block = editor()->basic_block();
  auto const merge_block = EmitMergeBlock();

  auto const true_block = editor()->EditNewBasicBlock(merge_block);
  auto const true_value = GenerateValue(node->true_expression());
  editor()->SetBranch(merge_block);
  Commit();

  auto const false_block = editor()->EditNewBasicBlock(merge_block);
  auto const false_value = GenerateValue(node->false_expression());
  editor()->SetBranch(merge_block);
  Commit();

  DCHECK_EQ(true_value->type(), false_value->type());

  editor()->Continue(cond_block);
  editor()->SetBranch(cond_value, true_block, false_block);
  Commit();

  editor()->Edit(merge_block);
  if (!NeedOutput())
    return;

  auto const phi = editor()->NewPhi(true_value->type());
  editor()->SetPhiInput(phi, true_block, true_value);
  editor()->SetPhiInput(phi, false_block, false_value);
  EmitOutput(phi);
}

void CodeGenerator::VisitLiteral(ast::Literal* node) {
  if (!NeedOutput())
    return;
  auto const value = ValueOf(node)->as<ir::Literal>();
  EmitOutput(NewLiteral(MapType(value->type()), node->token()));
}

void CodeGenerator::VisitNameReference(ast::NameReference* node) {
  if (!NeedOutput())
    return;
  auto const value = ValueOf(node);
  // TODO(eval1749) |value| can be
  //    |ir::Class| load class object literal
  //    |ir::Field| load instance or static field
  //    |ir::Literal| constant variable reference, or enum member.
  if (auto const method = value->as<ir::Method>()) {
    auto const method_name =
        factory()->NewAtomicString(method->ast_method()->NewQualifiedName());
    EmitOutput(
        factory()->NewReference(MapType(method->signature()), method_name));
    return;
  }
  NOTREACHED() << "Unsupported value " << *value;
}

void CodeGenerator::VisitParameterReference(ast::ParameterReference* node) {
  EmitVariableReference(node->parameter());
}

void CodeGenerator::VisitVariableReference(ast::VariableReference* node) {
  EmitVariableReference(node->variable());
}

//
// ast::Visitor statement nodes
//
void CodeGenerator::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements()) {
    if (!editor()->basic_block()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
    Generate(statement);
  }
}

void CodeGenerator::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  editor()->SetBranch(break_context_->break_block);
  Commit();
}

void CodeGenerator::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  DCHECK(break_context_->continue_block);
  editor()->SetBranch(break_context_->continue_block);
  Commit();
}

void CodeGenerator::VisitDoStatement(ast::DoStatement* node) {
  GenerateDoOrWhile(node);
}

void CodeGenerator::VisitExpressionList(ast::ExpressionList* node) {
  for (auto const expression : node->expressions())
    expression->Accept(this);
}

void CodeGenerator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  DCHECK(!output_);
  node->expression()->Accept(this);
}

// VisitForStatement generates following blocks:
//    ... initializer...
//    br while
//   loop:
//    ...
//    br continue
//   continue:
//    ... step ...
//    br while
//   while:
//    ... condition ...
//    br %condition, loop, break
//   break:
//    ...
void CodeGenerator::VisitForStatement(ast::ForStatement* node) {
  if (!node->step()) {
    GenerateDoOrWhile(node);
    return;
  }
  auto const head_block = editor()->basic_block();
  auto const break_block = EmitMergeBlock();

  auto const while_block = editor()->NewBasicBlock(break_block);
  auto const loop_block = editor()->NewBasicBlock(while_block);
  auto const continue_block = editor()->NewBasicBlock(while_block);

  editor()->Continue(head_block);
  editor()->SetBranch(while_block);
  Generate(node->initializer());
  Commit();

  editor()->Edit(loop_block);
  editor()->SetBranch(continue_block);
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    Generate(node->statement());
  }
  Commit();

  editor()->Edit(continue_block);
  editor()->SetBranch(while_block);
  Generate(node->step());
  Commit();

  editor()->Edit(while_block);
  editor()->SetBranch(GenerateBool(node->condition()), loop_block, break_block);
  Commit();
  editor()->Edit(break_block);
}

void CodeGenerator::VisitIfStatement(ast::IfStatement* node) {
  auto const cond_value = GenerateBool(node->condition());
  auto const cond_block = editor()->basic_block();
  auto const merge_block = EmitMergeBlock();

  auto const then_block = editor()->EditNewBasicBlock(merge_block);
  Generate(node->then_statement());
  if (editor()->basic_block()) {
    editor()->SetBranch(merge_block);
    Commit();
  }

  auto else_block = merge_block;
  if (node->else_statement()) {
    else_block = editor()->EditNewBasicBlock(merge_block);
    Generate(node->else_statement());
    if (editor()->basic_block()) {
      editor()->SetBranch(merge_block);
      Commit();
    }
  }

  editor()->Continue(cond_block);
  editor()->SetBranch(cond_value, then_block, else_block);
  Commit();
  editor()->Edit(merge_block);
}

void CodeGenerator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const return_value =
      node->value() ? GenerateValue(node->value()) : void_value();
  editor()->SetReturn(return_value);
  Commit();
}

void CodeGenerator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const ast_variable : node->variables())
    EmitVariableBinding(ast_variable, ast_variable->value(), nullptr);
}

void CodeGenerator::VisitWhileStatement(ast::WhileStatement* node) {
  GenerateDoOrWhile(node);
}

}  // namespace compiler
}  // namespace elang
