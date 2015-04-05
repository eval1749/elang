// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/query/node_queries.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

namespace {
bool MatchName(AtomicString* name, Token* token) {
  if (!name)
    return true;
  if (!token->has_atomic_string())
    return false;
  return name == token->atomic_string();
}
}  // namespace

// QueryContext
struct QueryContext : Visitor {
  std::vector<Node*> nodes;
  const NodeQuery* query;
  CompilationSession* session;

 private:
  // Visitor
  void DoDefaultVisit(Node* node);
  void VisitMethodGroup(MethodGroup* node);
  void VisitMethod(Method* node);

  // Visitor statements
  void VisitBlockStatement(BlockStatement* node);
  void VisitExpressionList(ExpressionList* node);
  void VisitExpressionStatement(ExpressionStatement* node);
  void VisitVarStatement(VarStatement* node);
};

void QueryContext::DoDefaultVisit(Node* node) {
  if (query->Match(this, node))
    nodes.push_back(node);
  auto const container = node->as<ContainerNode>();
  if (!container)
    return;
  for (auto const it : container->named_members())
    Visit(it.second);
}

void QueryContext::VisitMethodGroup(MethodGroup* node) {
  DoDefaultVisit(node);
  for (auto const method : node->methods())
    Visit(method);
}

void QueryContext::VisitMethod(Method* node) {
  DoDefaultVisit(node);
  Visit(node->return_type());
  for (auto const parameter : node->parameters())
    Visit(parameter);
  if (!node->body())
    return;
  Visit(node->body());
}

// Visitor statements
void QueryContext::VisitBlockStatement(BlockStatement* node) {
  DoDefaultVisit(node);
  for (auto const statement : node->statements())
    Visit(statement);
}

void QueryContext::VisitExpressionList(ExpressionList* node) {
  for (auto const expression : node->expressions())
    Visit(expression);
}

void QueryContext::VisitExpressionStatement(ExpressionStatement* node) {
  Visit(node->expression());
}

void QueryContext::VisitVarStatement(VarStatement* node) {
  DoDefaultVisit(node);
  for (auto const var_decl : node->variables()) {
    Visit(var_decl->variable());
    Visit(var_decl->value());
  }
}

// NameQuery
NameQuery::NameQuery(AtomicString* name) : name_(name) {
}

NameQuery::~NameQuery() {
}

bool NameQuery::Match(QueryContext* context, Node* node) const {
  auto const named = node->as<NamedNode>();
  return named && named->name()->atomic_string() == name_;
}

// NodeQuery
NodeQuery::NodeQuery() {
}

NodeQuery::~NodeQuery() {
}

// OrQuery
OrQuery::OrQuery(const std::vector<NodeQuery*>& queries) : queries_(queries) {
  DCHECK(!queries_.empty());
}

OrQuery::~OrQuery() {
}

bool OrQuery::Match(QueryContext* context, Node* node) const {
  for (auto const query : queries_) {
    if (query->Match(context, node))
      return true;
  }
  return false;
}

// TokenTypeQuery
TokenTypeQuery::TokenTypeQuery(TokenType token_type) : token_type_(token_type) {
}

TokenTypeQuery::~TokenTypeQuery() {
}

bool TokenTypeQuery::Match(QueryContext* context, Node* node) const {
  return node->token() == token_type_;
}

}  // namespace ast

// CompilationSession
std::vector<ast::Node*> CompilationSession::QueryAstNodes(
    const ast::NodeQuery& query) {
  ast::QueryContext context;
  context.query = &query;
  context.session = this;
  context.Visit(global_namespace());
  return context.nodes;
}

}  // namespace compiler
}  // namespace elang
