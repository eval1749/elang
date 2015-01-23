// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/code_generator.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/cg/type_mapper.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/intrinsic_names.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::Output
//
struct CodeGenerator::Output {
  hir::Instruction* instruction;
  int position;
  hir::Type* type;
  hir::Value* value;

  // |position| is -1 means current context expects value rather than setting
  // value into instruction.
  // |type| is |VoidType| means current context expects no output, e.g. middle
  // of block statement or `return` statement for `void` method.
  Output(hir::Type* type, hir::Instruction* instruction, int position)
      : instruction(instruction),
        position(position),
        type(type),
        value(nullptr) {
    DCHECK(instruction);
    if (type->is<hir::VoidType>())
      return;
    DCHECK_GE(position, -1);
    DCHECK_LT(position, instruction->CountOperands());
  }
};

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::ScopedOutput
// To set output of current context for visitor.
//
class CodeGenerator::ScopedOutput {
 public:
  ScopedOutput(CodeGenerator* generator,
               hir::Type* type,
               hir::Instruction* instruction,
               int position);
  ~ScopedOutput();

 private:
  CodeGenerator* const generator_;
  Output output_;
  Output* const previous_output_;

  DISALLOW_COPY_AND_ASSIGN(ScopedOutput);
};

CodeGenerator::ScopedOutput::ScopedOutput(CodeGenerator* generator,
                                          hir::Type* type,
                                          hir::Instruction* instruction,
                                          int position)
    : generator_(generator),
      output_(type, instruction, position),
      previous_output_(generator->output_) {
  generator_->output_ = &output_;
}

CodeGenerator::ScopedOutput::~ScopedOutput() {
  generator_->output_ = previous_output_;
}

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
CodeGenerator::CodeGenerator(CompilationSession* session, hir::Factory* factory)
    : CompilationSessionUser(session),
      editor_(nullptr),
      factory_(factory),
      function_(nullptr),
      output_(nullptr),
      type_mapper_(new TypeMapper(session, factory)),
      void_type_(MapType(PredefinedName::Void)) {
}

CodeGenerator::~CodeGenerator() {
}

hir::TypeFactory* CodeGenerator::types() const {
  return factory()->types();
}

void CodeGenerator::Emit(hir::Instruction* instruction) {
  DCHECK(output_);
  editor_->Append(instruction);
}

void CodeGenerator::EmitOutput(hir::Instruction* instruction) {
  if (!instruction->id())
    Emit(instruction);
  if (!NeedOutput()) {
    DCHECK(!instruction->CanBeRemoved());
    return;
  }
  EmitOutput(static_cast<hir::Value*>(instruction));
}

void CodeGenerator::EmitOutput(hir::Value* value) {
  DCHECK(output_);
  DCHECK(NeedOutput());
  if (output_->type == void_type())
    return;
  if (output_->position < 0) {
    output_->value = value;
    return;
  }
  editor_->SetInput(output_->instruction, output_->position, value);
}

void CodeGenerator::EmitParameterBindings(ast::Method* ast_method) {
  if (ast_method->parameters().empty())
    return;
  if (ast_method->parameters().size() == 1u) {
    EmitVariableBinding(ast_method->parameters()[0], nullptr,
                        function_->entry_block()->first_instruction());
    return;
  }
  NOTREACHED() << "NYI multiple parameters" << *ast_method;
}

void CodeGenerator::EmitVariableBinding(ast::NamedNode* ast_variable,
                                        ast::Expression* ast_value,
                                        hir::Value* value) {
  auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
  auto const variable_type = MapType(variable->type());

  if (variable->storage() == ir::StorageClass::Void) {
    if (!ast_value)
      return;
    GenerateValue(void_type(), ast_value);
    return;
  }

  if (variable->storage() == ir::StorageClass::ReadOnly) {
    DCHECK(!variables_.count(variable));
    if (value) {
      variables_[variable] = value;
      return;
    }
    if (ast_value) {
      variables_[variable] = GenerateValue(variable_type, ast_value);
      return;
    }
    variables_[variable] = variable_type->default_value();
    return;
  }

  DCHECK_EQ(variable->storage(), ir::StorageClass::Local);
  auto const pointer_type = types()->NewPointerType(variable_type);
  auto const allocator_type =
      types()->NewFunctionType(pointer_type, void_type());
  auto const allocator = factory()->NewReference(
      allocator_type,
      factory()->intrinsic_name(hir::IntrinsicName::StackAlloc));
  auto const alloc_instr =
      factory()->NewCallInstruction(allocator, factory()->void_value());
  DCHECK(!variables_.count(variable));
  variables_[variable] = alloc_instr;
  Emit(alloc_instr);
  auto const store_instr = factory()->NewStoreInstruction(
      alloc_instr, value ? value : variable_type->default_value());
  Emit(store_instr);
  if (!ast_value)
    return;
  ScopedOutput bind_scope(this, variable_type, store_instr, 1);
  ast_value->Accept(this);
}

void CodeGenerator::EmitVariableReference(ast::NamedNode* ast_variable) {
  if (!NeedOutput())
    return;
  auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
  DCHECK(variable);
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end());
  if (variable->storage() == ir::StorageClass::ReadOnly) {
    EmitOutput(it->second);
    return;
  }
  EmitOutput(factory()->NewLoadInstruction(it->second));
}

hir::Function* CodeGenerator::FunctionOf(ast::Method* ast_method) const {
  auto const it = functions_.find(ast_method);
  return it == functions_.end() ? nullptr : it->second;
}

hir::Value* CodeGenerator::GenerateValue(hir::Type* type,
                                         ast::Expression* expression) {
  ScopedOutput output(this, type, output_->instruction, -1);
  expression->Accept(this);
  DCHECK(output_->value);
  return output_->value;
}

hir::Type* CodeGenerator::MapType(PredefinedName name) const {
  return type_mapper_->Map(name);
}

hir::Type* CodeGenerator::MapType(ir::Type* type) const {
  return type_mapper_->Map(type);
}

bool CodeGenerator::NeedOutput() const {
  DCHECK(output_);
  return output_->type && output_->type != void_type();
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
  function_ = factory()->NewFunction(
      type_mapper()->Map(method->signature())->as<hir::FunctionType>());
  hir::Editor editor(factory(), function_);
  editor_ = &editor;
  EmitParameterBindings(ast_method);
  auto const return_instr = function_->entry_block()->last_instruction();
  editor.Edit(return_instr->basic_block());
  ScopedOutput scoped_output(this, void_type(), return_instr, 0);
  ast_method->body()->Accept(this);
  // TODO(eval1749) We should check |return_instr| has value if it is reachable.
  functions_[ast_method] = function_;
  editor.Commit();
  editor_ = nullptr;
  function_ = nullptr;
}

//
// ast::Visitor expression nodes
//

void CodeGenerator::VisitCall(ast::Call* node) {
  DCHECK(node);
  auto const callee = ValueOf(node->callee())->as<ir::Method>();
  DCHECK(callee) << "Unresolved call" << *node;
  auto const callee_type =
      MapType(callee->signature())->as<hir::FunctionType>();
  // TODO(eval1749) We should make 'call' instruction takes multiple
  // operands.
  auto const call_instr = factory()->NewCallInstruction(
      callee_type->default_value(),
      MapType(PredefinedName::Void)->default_value());
  editor_->InsertBefore(call_instr, output_->instruction);

  // Generate argument list.
  auto position = static_cast<int>(node->arguments().size());
  auto arguments = node->arguments().rbegin();
  while (position) {
    auto const arg_type = callee_type->parameters_type();
    ScopedOutput output_to_argument(this, arg_type, call_instr, position);
    (*arguments)->Accept(this);
    --position;
    ++arguments;
  }
  // Generate callee
  {
    ScopedOutput output_callee(this, callee_type, call_instr, 0);
    node->callee()->Accept(this);
  }
  // Set call value if needed
  if (output_->type == void_type()) {
    if (call_instr->CanBeRemoved())
      editor_->RemoveInstruction(call_instr);
    return;
  }
  EmitOutput(call_instr);
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
    if (statement == node->statements().back()) {
      statement->Accept(this);
      break;
    }
    // Interleaved statement has no output.
    ScopedOutput scoped_output(this, void_type(), output_->instruction, -1);
    statement->Accept(this);
    if (!output_) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
  }
}

void CodeGenerator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  node->expression()->Accept(this);
}

void CodeGenerator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const return_type = function_->return_type();
  auto const ast_value = node->value();
  if (output_->instruction->is<hir::ReturnInstruction>()) {
    if (!ast_value)
      return;
    output_->type = return_type;
    ast_value->Accept(this);
    output_ = nullptr;
    return;
  }
  auto const return_instr = factory()->NewReturnInstruction(
      return_type->default_value(), function_->exit_block());
  {
    ScopedOutput return_scope(this, return_type, return_instr, 0);
    ast_value->Accept(this);
  }
  output_ = nullptr;
}

void CodeGenerator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const ast_variable : node->variables())
    EmitVariableBinding(ast_variable, ast_variable->value(), nullptr);
}

}  // namespace compiler
}  // namespace elang
