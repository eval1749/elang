// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/analyze/method_analyzer.h"

#include "base/logging.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/analyze/type_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"

namespace elang {
namespace compiler {

namespace {

//////////////////////////////////////////////////////////////////////
//
// MethodBodyAnalyzer
//
class MethodBodyAnalyzer final : public Analyzer, private ast::Visitor {
 public:
  MethodBodyAnalyzer(NameResolver* name_resolver, ast::Method* method);
  ~MethodBodyAnalyzer() final = default;

  // The Entry Point of |MethodBodyAnalyzer|
  void Run();

 private:
  // ast::Visitor
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;

  ast::Method* const method_;
  TypeResolver* const type_resolver_;

  DISALLOW_COPY_AND_ASSIGN(MethodBodyAnalyzer);
};

//////////////////////////////////////////////////////////////////////
//
// MethodBodyAnalyzer
// Traversal statements in method body.
//
MethodBodyAnalyzer::MethodBodyAnalyzer(NameResolver* name_resolver,
                                       ast::Method* method)
    : Analyzer(name_resolver),
      method_(method),
      type_resolver_(new TypeResolver(name_resolver, method)) {
}

// The entry point of |MethodBodyAnalyzer|.
void MethodBodyAnalyzer::Run() {
  auto const ir_method = resolver()->Resolve(method_);
  if (!ir_method)
    return;
  auto const body = method_->body();
  if (!body)
    return;
  body->Accept(this);
}

// ast::Visitor
void MethodBodyAnalyzer::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements())
    statement->Accept(this);
}

void MethodBodyAnalyzer::VisitExpressionStatement(
    ast::ExpressionStatement* node) {
  type_resolver_->Unify(node->expression(), type_resolver_->GetAnyValue());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// MethodAnalyzer
//
MethodAnalyzer::MethodAnalyzer(NameResolver* resolver) : Analyzer(resolver) {
}

MethodAnalyzer::~MethodAnalyzer() {
}

// The entry point of |MethodAnalyzer|.
bool MethodAnalyzer::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

// ast::Visitor
void MethodAnalyzer::VisitClass(ast::Class* node) {
  node->AcceptForMembers(this);
}

void MethodAnalyzer::VisitMethod(ast::Method* method) {
  MethodBodyAnalyzer method_resolver(resolver(), method);
  method_resolver.Run();
}

void MethodAnalyzer::VisitNamespaceBody(ast::NamespaceBody* node) {
  node->AcceptForMembers(this);
}

}  // namespace compiler
}  // namespace elang
