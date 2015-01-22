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

void CodeGenerator::SetOutput(hir::Instruction* instruction) {
  if (!instruction->id())
    editor_->Append(instruction);
  if (!NeedOutput()) {
    DCHECK(!instruction->CanBeRemoved());
    return;
  }
  SetOutput(static_cast<hir::Value*>(instruction));
}

void CodeGenerator::SetOutput(hir::Value* value) {
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
  auto const return_instr = function_->entry_block()->last_instruction();
  editor.Edit(return_instr->basic_block());
  auto const return_type = return_instr->OperandAt(0)->type();
  ScopedOutput scoped_output(this, return_type, return_instr, 0);
  ast_method->body()->Accept(this);
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
      callee_type->GetDefaultValue(),
      MapType(PredefinedName::Void)->GetDefaultValue());
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
  SetOutput(call_instr);
}

void CodeGenerator::VisitLiteral(ast::Literal* node) {
  if (!NeedOutput())
    return;
  auto const value = ValueOf(node)->as<ir::Literal>();
  SetOutput(NewLiteral(MapType(value->type()), node->token()));
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
    SetOutput(
        factory()->NewReference(MapType(method->signature()),
                                method->ast_method()->NewQualifiedName()));
    return;
  }
  NOTREACHED() << "Unsupported value " << *value;
}

void CodeGenerator::VisitVariableReference(ast::VariableReference* node) {
  if (!NeedOutput())
    return;
  auto const ast_variable = node->variable();
  auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
  DCHECK(variable);
  auto const it = variables_.find(variable);
  DCHECK(it != variables_.end());
  if (variable->storage() == ir::StorageClass::ReadOnly) {
    SetOutput(it->second);
    return;
  }
  SetOutput(factory()->NewLoadInstruction(it->second));
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
    if (statement->IsTerminator()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
  }
}

void CodeGenerator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  node->expression()->Accept(this);
}

void CodeGenerator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const ast_variable : node->variables()) {
    auto const variable = ValueOf(ast_variable)->as<ir::Variable>();
    DCHECK(variable);
    auto const variable_type = MapType(variable->type());

    if (variable->storage() == ir::StorageClass::ReadOnly) {
      DCHECK(!variables_.count(variable));
      if (auto const ast_expression = ast_variable->value())
        variables_[variable] = GenerateValue(variable_type, ast_expression);
      else
        variables_[variable] = variable_type->GetDefaultValue();
      continue;
    }

    DCHECK_EQ(variable->storage(), ir::StorageClass::Local);
    auto const pointer_type = types()->NewPointerType(variable_type);
    auto const allocator_type =
        types()->NewFunctionType(pointer_type, void_type());
    auto const allocator = factory()->NewReference(
        allocator_type,
        factory()->intrinsic_name(hir::IntrinsicName::StackAlloc));
    auto const pointer =
        factory()->NewCallInstruction(allocator, factory()->void_value());
    DCHECK(!variables_.count(variable));
    variables_[variable] = pointer;
    editor_->Append(pointer);
    if (!ast_variable->value())
      return;
    auto const store_instr = factory()->NewStoreInstruction(
        pointer, variable_type->GetDefaultValue());
    ScopedOutput bind_scope(this, variable_type, store_instr, 1);
    ast_variable->value()->Accept(this);
  }
}

}  // namespace compiler
}  // namespace elang
