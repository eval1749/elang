// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/code_generator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
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
      DCHECK_LE(position, 0);
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
CodeGenerator::CodeGenerator(CompilationSession* session,
                             hir::Factory* factory,
                             NameResolver* name_resolver)
    : editor_(nullptr),
      factory_(factory),
      function_(nullptr),
      name_resolver_(name_resolver),
      output_(nullptr),
      session_(session),
      type_mapper_(new TypeMapper(session, factory)) {
}

CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::Generate() {
  session_->global_namespace()->AcceptForMembers(this);
}

Semantics* CodeGenerator::semantics() const {
  return session_->semantics();
}

hir::Type* CodeGenerator::MapType(PredefinedName name) {
  return type_mapper_->Map(name);
}

hir::Type* CodeGenerator::MapType(ir::Type* type) {
  return type_mapper_->Map(type);
}

// ast::Visitor

// Declaration nodes
void CodeGenerator::VisitAlias(ast::Alias* node) {
  DCHECK(node);
}

void CodeGenerator::VisitClass(ast::Class* clazz) {
  DCHECK(clazz);
  NOTREACHED();
}

void CodeGenerator::VisitClassBody(ast::ClassBody* node) {
  node->AcceptForMembers(this);
}

void CodeGenerator::VisitEnum(ast::Enum* node) {
  DCHECK(node);
}

void CodeGenerator::VisitField(ast::Field* node) {
  DCHECK(node);
}

void CodeGenerator::VisitImport(ast::Import* node) {
  DCHECK(node);
}

void CodeGenerator::VisitMethod(ast::Method* ast_method) {
  DCHECK(!editor_);
  DCHECK(!function_);
  //  1 Convert ast::FunctionType to hir::FunctionType
  //  2 hir::NewFunction(function_type)
  auto const method = name_resolver()->Resolve(ast_method)->as<ir::Method>();
  if (!method) {
    DVLOG(0) << "Not resolved " << *ast_method;
    return;
  }
  function_ = factory()->NewFunction(
      type_mapper()->Map(method->signature())->as<hir::FunctionType>());
  hir::Editor editor(factory(), function_);
  auto const return_instr = function_->entry_block()->last_instruction();
  ScopedOutput scoped_output(this, return_instr->OperandAt(0)->type(),
                             return_instr, 0);
  ast_method->body()->Accept(this);
  methods_[ast_method] = function_;
  editor_ = nullptr;
  function_ = nullptr;
}

void CodeGenerator::VisitNamespace(ast::Namespace* node) {
  DCHECK(node);
  NOTREACHED();
}

void CodeGenerator::VisitNamespaceBody(ast::NamespaceBody* node) {
  node->AcceptForMembers(this);
}

// Expression nodes
void CodeGenerator::VisitArrayType(ast::ArrayType* node) {
  DCHECK(node);
}

void CodeGenerator::VisitAssignment(ast::Assignment* assignment) {
  DCHECK(assignment);
}

void CodeGenerator::VisitBinaryOperation(ast::BinaryOperation* node) {
  DCHECK(node);
}

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
  ScopedOutput output_callee(this, callee_type, call_instr, 0);
  node->callee()->Accept(this);
}

void CodeGenerator::VisitConditional(ast::Conditional* node) {
  DCHECK(node);
}

void CodeGenerator::VisitConstructedType(ast::ConstructedType* node) {
  DCHECK(node);
}

void CodeGenerator::VisitInvalidExpression(ast::InvalidExpression* node) {
  DCHECK(node);
}

void CodeGenerator::VisitLiteral(ast::Literal* node) {
  DCHECK(node);
}

void CodeGenerator::VisitMemberAccess(ast::MemberAccess* node) {
  DCHECK(node);
}

void CodeGenerator::VisitNameReference(ast::NameReference* node) {
  DCHECK(node);
  // TODO(eval1749) We need to have
  // |NameResolver::GetReference(ast::Expression*)|.
}

void CodeGenerator::VisitUnaryOperation(ast::UnaryOperation* node) {
  DCHECK(node);
}

void CodeGenerator::VisitVariableReference(ast::VariableReference* node) {
  DCHECK(node);
}

// Statement nodes
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

void CodeGenerator::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitDoStatement(ast::DoStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitEmptyStatement(ast::EmptyStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  node->expression()->Accept(this);
}

void CodeGenerator::VisitExpressionList(ast::ExpressionList* node) {
  DCHECK(node);
}

void CodeGenerator::VisitForEachStatement(ast::ForEachStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitForStatement(ast::ForStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitIfStatement(ast::IfStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitInvalidStatement(ast::InvalidStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitReturnStatement(ast::ReturnStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitThrowStatement(ast::ThrowStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitTryStatement(ast::TryStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitUsingStatement(ast::UsingStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitVarStatement(ast::VarStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitWhileStatement(ast::WhileStatement* node) {
  DCHECK(node);
}

void CodeGenerator::VisitYieldStatement(ast::YieldStatement* node) {
  DCHECK(node);
}

}  // namespace compiler
}  // namespace elang
