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
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
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
  // When |type| is |nullptr|, no output is expected. It is different from
  // |void| type. In this case, |position| must be -1.
  hir::Type* type;

  Output(hir::Type* type, hir::Instruction* instruction, int position)
      : instruction(instruction), position(position), type(type) {
    DCHECK(instruction);
    if (type) {
      DCHECK_GE(position, 0);
    } else {
      DCHECK_EQ(position, -1);
    }
  }
};

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator::ScopedOutput
//
class CodeGenerator::ScopedOutput {
 public:
  ScopedOutput(CodeGenerator* generator,
               hir::Type* type,
               hir::Instruction* instruction,
               int position);
  ScopedOutput(CodeGenerator* generator, hir::Instruction* instruction);
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

CodeGenerator::ScopedOutput::ScopedOutput(CodeGenerator* generator,
                                          hir::Instruction* instruction)
    : ScopedOutput(generator, nullptr, instruction, -1) {
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

hir::Function* CodeGenerator::FunctionOf(ast::Method* ast_method) const {
  auto const it = functions_.find(ast_method);
  return it == functions_.end() ? nullptr : it->second;
}

hir::Type* CodeGenerator::MapType(PredefinedName name) const {
  return type_mapper_->Map(name);
}

hir::Type* CodeGenerator::MapType(ir::Type* type) const {
  return type_mapper_->Map(type);
}

// The entry point of |CodeGenerator|.
bool CodeGenerator::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

void CodeGenerator::SetOutput(hir::Value* value) {
  DCHECK(output_);
  DCHECK_GE(output_->position, 0);
  if (output_->type == void_type())
    return;
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
  auto const method = semantics()->ValueOf(ast_method)->as<ir::Method>();
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
  ScopedOutput scoped_output(this, return_instr->OperandAt(0)->type(),
                             return_instr, 0);
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
  auto const callee = semantics()->ValueOf(node->callee())->as<ir::Method>();
  DCHECK(callee) << "Unresolved call" << *node;
  auto const callee_type =
      MapType(callee->signature())->as<hir::FunctionType>();
  // TODO(eval1749) We should make 'call' instruction takes multiple
  // operands.
  auto const call_instr = hir::CallInstruction::New(
      factory(), output_->type, callee_type->GetDefaultValue(),
      MapType(PredefinedName::Void)->GetDefaultValue());
  editor_->InsertBefore(call_instr, output_->instruction);
  auto position = static_cast<int>(node->arguments().size());
  auto arguments = node->arguments().rbegin();
  while (position) {
    auto const arg_type = callee_type->parameters_type();
    ScopedOutput output_to_argument(this, arg_type, call_instr, position);
    (*arguments)->Accept(this);
    --position;
    ++arguments;
  }
  {
    ScopedOutput output_callee(this, callee_type, call_instr, 0);
    node->callee()->Accept(this);
  }
  SetOutput(call_instr);
}

void CodeGenerator::VisitNameReference(ast::NameReference* node) {
  auto const value = ValueOf(node);
  // TODO(eval1749) |value| can be
  //    |ir::Class| load class object literal
  //    |ir::Field| load instance or static field
  //    |ir::Literal| constant variable reference, or enum member.
  //    |ir::Variable| load value of variable.
  if (auto const method = value->as<ir::Method>()) {
    SetOutput(
        factory()->NewReference(MapType(method->signature()),
                                method->ast_method()->NewQualifiedName()));
    return;
  }
  NOTREACHED() << "Unsupported value " << *value;
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
    ScopedOutput scoped_output(this, output_->instruction);
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
}  // namespace compiler
}  // namespace elang
