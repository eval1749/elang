// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/code_generator.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
CodeGenerator::CodeGenerator(CompilationSession* session,
                             hir::Factory* factory,
                             NameResolver* name_resolver)
    : factory_(factory),
      function_(nullptr),
      name_resolver_(name_resolver),
      session_(session) {
}

CodeGenerator::~CodeGenerator() {
}

void CodeGenerator::Generate() {
  session_->global_namespace()->AcceptForMembers(this);
}

// ast::Visitor

// Declaration nodes
void CodeGenerator::VisitAlias(ast::Alias* node) {
  DCHECK(node);
}

void CodeGenerator::VisitClass(ast::Class* clazz) {
  clazz->AcceptForMembers(this);
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

void CodeGenerator::VisitMethod(ast::Method* method) {
  DCHECK(!function_);
  // TODO(eval1749) NYI do following steps:
  //  1 Convert ast::FunctionType to hir::FunctionType
  //  2 NewFunction()
  method->body()->Accept(this);
  methods_[method] = function_;
  function_ = nullptr;
}

void CodeGenerator::VisitNamespace(ast::Namespace* node) {
  DCHECK(node);
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
}

void CodeGenerator::VisitUnaryOperation(ast::UnaryOperation* node) {
  DCHECK(node);
}

void CodeGenerator::VisitVariableReference(ast::VariableReference* node) {
  DCHECK(node);
}

// Statement nodes
void CodeGenerator::VisitBlockStatement(ast::BlockStatement* node) {
  DCHECK(node);
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
  DCHECK(node);
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
