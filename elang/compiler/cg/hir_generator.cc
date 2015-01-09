// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/hir_generator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// HirGenerator
//
HirGenerator::HirGenerator(CompilationSession* session,
                           hir::Factory* factory,
                           NameResolver* name_resolver)
    : factory_(factory),
      function_(nullptr),
      name_resolver_(name_resolver),
      session_(session) {
}

HirGenerator::~HirGenerator() {
}

void HirGenerator::Generate() {
  ProcessMemberContainer(session_->global_namespace());
}

void HirGenerator::ProcessMemberContainer(ast::MemberContainer* container) {
  for (auto const name_member : container->name_map())
    name_member.second->Accept(this);
}

// ast::Visitor

// Declaration nodes
void HirGenerator::VisitAlias(ast::Alias* node) {
  DCHECK(node);
}

void HirGenerator::VisitClass(ast::Class* clazz) {
  ProcessMemberContainer(clazz);
}

void HirGenerator::VisitEnum(ast::Enum* node) {
  DCHECK(node);
}

void HirGenerator::VisitField(ast::Field* node) {
  DCHECK(node);
}

void HirGenerator::VisitImport(ast::Import* node) {
  DCHECK(node);
}

void HirGenerator::VisitMethod(ast::Method* method) {
  DCHECK(!function_);
  // TODO(eval1749) NYI do following steps:
  //  1 Convert ast::FunctionType to hir::FunctionType
  //  2 NewFunction()
  method->statement()->Accept(this);
  methods_[method] = function_;
  function_ = nullptr;
}

void HirGenerator::VisitMethodGroup(ast::MethodGroup* method_group) {
  for (auto const method : method_group->methods())
    VisitMethod(method);
}

void HirGenerator::VisitNamespace(ast::Namespace* namespaze) {
  ProcessMemberContainer(namespaze);
}

// Expression nodes
void HirGenerator::VisitArrayType(ast::ArrayType* node) {
  DCHECK(node);
}

void HirGenerator::VisitAssignment(ast::Assignment* assignment) {
  DCHECK(assignment);
}

void HirGenerator::VisitBinaryOperation(ast::BinaryOperation* node) {
  DCHECK(node);
}

void HirGenerator::VisitCall(ast::Call* node) {
  DCHECK(node);
}

void HirGenerator::VisitConditional(ast::Conditional* node) {
  DCHECK(node);
}

void HirGenerator::VisitConstructedType(ast::ConstructedType* node) {
  DCHECK(node);
}

void HirGenerator::VisitInvalidExpression(ast::InvalidExpression* node) {
  DCHECK(node);
}

void HirGenerator::VisitLiteral(ast::Literal* node) {
  DCHECK(node);
}

void HirGenerator::VisitMemberAccess(ast::MemberAccess* node) {
  DCHECK(node);
}

void HirGenerator::VisitNameReference(ast::NameReference* node) {
  DCHECK(node);
}

void HirGenerator::VisitUnaryOperation(ast::UnaryOperation* node) {
  DCHECK(node);
}

void HirGenerator::VisitVariableReference(ast::VariableReference* node) {
  DCHECK(node);
}

// Statement nodes
void HirGenerator::VisitBlockStatement(ast::BlockStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitDoStatement(ast::DoStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitEmptyStatement(ast::EmptyStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitExpressionList(ast::ExpressionList* node) {
  DCHECK(node);
}

void HirGenerator::VisitForEachStatement(ast::ForEachStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitForStatement(ast::ForStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitIfStatement(ast::IfStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitInvalidStatement(ast::InvalidStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitReturnStatement(ast::ReturnStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitThrowStatement(ast::ThrowStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitTryStatement(ast::TryStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitUsingStatement(ast::UsingStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitVarStatement(ast::VarStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitWhileStatement(ast::WhileStatement* node) {
  DCHECK(node);
}

void HirGenerator::VisitYieldStatement(ast::YieldStatement* node) {
  DCHECK(node);
}

}  // namespace compiler
}  // namespace elang
