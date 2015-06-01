// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_QUERY_NODE_QUERIES_H_
#define ELANG_COMPILER_AST_QUERY_NODE_QUERIES_H_

#include <string>
#include <vector>

#include "base/basictypes.h"

namespace elang {
class AtomicString;
namespace compiler {
class CompilationSession;
enum class TokenType;
namespace ast {

class Node;
struct QueryContext;

// NodeQuery - base class of AST tree query expression.
class NodeQuery {
 public:
  virtual bool Match(QueryContext* context, ast::Node* node) const = 0;

 protected:
  NodeQuery();
  virtual ~NodeQuery();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeQuery);
};

// NameQuery
class NameQuery : public NodeQuery {
 public:
  explicit NameQuery(AtomicString* name);
  ~NameQuery();

 private:
  bool Match(QueryContext* context, ast::Node* node) const final;

  AtomicString* name_;

  DISALLOW_COPY_AND_ASSIGN(NameQuery);
};

// OrQuery
class OrQuery : public NodeQuery {
 public:
  explicit OrQuery(const std::vector<NodeQuery*>& queries);
  ~OrQuery();

 private:
  bool Match(QueryContext* context, ast::Node* node) const final;

  const std::vector<NodeQuery*> queries_;

  DISALLOW_COPY_AND_ASSIGN(OrQuery);
};

// TokenTypeQuery
class TokenTypeQuery : public NodeQuery {
 public:
  explicit TokenTypeQuery(TokenType token_type);
  ~TokenTypeQuery();

 private:
  bool Match(QueryContext* context, ast::Node* node) const final;

  TokenType const token_type_;

  DISALLOW_COPY_AND_ASSIGN(TokenTypeQuery);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_QUERY_NODE_QUERIES_H_
