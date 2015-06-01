// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "elang/shell/node_query.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace shell {

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
struct QueryContext : ast::Visitor {
  std::vector<ast::Node*> nodes;
  const NodeQuery* query;
  CompilationSession* session;

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node);
};

void QueryContext::DoDefaultVisit(ast::Node* node) {
  if (query->Match(this, node))
    nodes.push_back(node);
  auto const container = node->as<ast::ContainerNode>();
  if (!container)
    return;
  for (auto const it : container->named_members())
    it.second->Accept(this);
}

// MethodQuery
MethodQuery::MethodQuery(AtomicString* name,
                         sm::Type* return_type,
                         const std::vector<ParameterQuery>& parameters)
    : name_(name), parameters_(parameters), return_type_(return_type) {
}

MethodQuery::~MethodQuery() {
}

bool MethodQuery::Match(QueryContext* context, ast::Node* node) const {
  auto const method =
      context->session->analysis()->SemanticOf(node)->as<sm::Method>();
  if (!method)
    return false;
  if (!MatchName(name_, method->name()))
    return false;
  if (return_type_ && method->return_type() != return_type_)
    return false;
  if (parameters_.empty())
    return method->parameters().empty();
  if (parameters_.size() != method->parameters().size())
    return false;
  auto it = parameters_.begin();
  for (auto const parameter : method->parameters()) {
    if (it->type && it->type != parameter->type())
      return false;
    if (!MatchName(it->name, parameter->name()))
      return false;
    ++it;
  }
  return true;
}

void MethodQuery::PrintTo(std::ostream* ostream) const {
  *ostream << "MethodQuery(";
  if (name_)
    *ostream << *name_;
  else
    *ostream << "_";
  *ostream << ", ";
  if (return_type_)
    *ostream << *return_type_;
  else
    *ostream << "_";
  *ostream << ", ";
  if (parameters_.empty()) {
    *ostream << "_";
  } else {
    *ostream << "{";
    for (auto const parameter : parameters_) {
      *ostream << "(";
      if (parameter.name)
        *ostream << *parameter.name;
      else
        *ostream << "_";
      *ostream << ", ";
      if (parameter.type)
        *ostream << *parameter.type;
      else
        *ostream << "_";
      *ostream << ")";
    }
    *ostream << "}";
  }
  *ostream << ")";
}

ParameterQuery::ParameterQuery(AtomicString* name, sm::Type* type)
    : name(name), type(type) {
}

ParameterQuery::ParameterQuery(sm::Type* type) : ParameterQuery(nullptr, type) {
}

ParameterQuery::ParameterQuery() : ParameterQuery(nullptr, nullptr) {
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

bool OrQuery::Match(QueryContext* context, ast::Node* node) const {
  for (auto const query : queries_) {
    if (query->Match(context, node))
      return true;
  }
  return false;
}

void OrQuery::PrintTo(std::ostream* ostream) const {
  *ostream << "OrQuery(";
  auto separator = "";
  for (auto const query : queries_) {
    *ostream << separator << query;
    separator = ", ";
  }
  *ostream << ")";
}

// QueryAllNodes
std::vector<ast::Node*> QueryAllNodes(CompilationSession* session,
                                      const NodeQuery* query) {
  QueryContext context;
  context.query = query;
  context.session = session;
  static_cast<ast::Node*>(session->global_namespace())->Accept(&context);
  return context.nodes;
}

std::ostream& operator<<(std::ostream& ostream, const NodeQuery& query) {
  query.PrintTo(&ostream);
  return ostream;
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
