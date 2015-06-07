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
  void DoDefaultVisit(Node* node) final;
  void VisitMethod(Method* node) final;

  // Visitor statements
  void VisitBlockStatement(BlockStatement* node) final;
  void VisitExpressionList(ExpressionList* node) final;
  void VisitExpressionStatement(ExpressionStatement* node) final;
  void VisitVarStatement(VarStatement* node) final;
};

void QueryContext::DoDefaultVisit(Node* node) {
  if (query->Match(this, node))
    nodes.push_back(node);
  auto const container = node->as<ContainerNode>();
  if (!container)
    return;
  for (auto const member : container->members())
    Traverse(member);
}

void QueryContext::VisitMethod(Method* node) {
  DoDefaultVisit(node);
  Traverse(node->return_type());
  for (auto const parameter : node->parameters())
    Traverse(parameter);
  if (!node->body())
    return;
  Traverse(node->body());
}

// Visitor statements
void QueryContext::VisitBlockStatement(BlockStatement* node) {
  DoDefaultVisit(node);
  for (auto const statement : node->statements())
    Traverse(statement);
}

void QueryContext::VisitExpressionList(ExpressionList* node) {
  for (auto const expression : node->expressions())
    Traverse(expression);
}

void QueryContext::VisitExpressionStatement(ExpressionStatement* node) {
  Traverse(node->expression());
}

void QueryContext::VisitVarStatement(VarStatement* node) {
  DoDefaultVisit(node);
  for (auto const var_decl : node->variables()) {
    Traverse(var_decl->variable());
    Traverse(var_decl->expression());
  }
}

// NameQuery
NameQuery::NameQuery(AtomicString* name) : name_(name) {
}

NameQuery::~NameQuery() {
}

bool NameQuery::Match(QueryContext* context, Node* node) const {
  return node->name()->is_name() && node->name()->atomic_string() == name_;
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
  Apply(&context);
  return context.nodes;
}

}  // namespace compiler
}  // namespace elang
