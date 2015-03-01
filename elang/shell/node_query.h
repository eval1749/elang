// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_NODE_QUERY_H_
#define ELANG_SHELL_NODE_QUERY_H_

#include <ostream>
#include <vector>

#include "base/basictypes.h"

namespace elang {
class AtomicString;
namespace compiler {
namespace ast {
class Node;
}
namespace ir {
class Type;
}
class CompilationSession;

namespace shell {

struct QueryContext;

// NodeQuery - base class of AST tree query expression.
class NodeQuery {
 public:
  virtual bool Match(QueryContext* context, ast::Node* node) const = 0;
  virtual void PrintTo(std::ostream* ostream) const = 0;

 protected:
  NodeQuery();
  virtual ~NodeQuery();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeQuery);
};

// ParameterQuery
struct ParameterQuery {
  AtomicString* name;
  ir::Type* type;
  ParameterQuery(AtomicString* name, ir::Type* type);
  explicit ParameterQuery(ir::Type* type);
  ParameterQuery();
};

// MethodQuery
class MethodQuery : public NodeQuery {
 public:
  MethodQuery(AtomicString* name,
              ir::Type* return_type,
              const std::vector<ParameterQuery>& parameters);
  ~MethodQuery();

 private:
  bool Match(QueryContext* context, ast::Node* node) const final;
  void PrintTo(std::ostream* ostream) const final;

  AtomicString* const name_;
  const std::vector<ParameterQuery> parameters_;
  ir::Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(MethodQuery);
};

// OrQuery
class OrQuery : public NodeQuery {
 public:
  explicit OrQuery(const std::vector<NodeQuery*>& queries);
  ~OrQuery();

 private:
  bool Match(QueryContext* context, ast::Node* node) const final;
  void PrintTo(std::ostream* ostream) const final;

  const std::vector<NodeQuery*> queries_;

  DISALLOW_COPY_AND_ASSIGN(OrQuery);
};

std::vector<ast::Node*> QueryAllNodes(CompilationSession* session,
                                      const NodeQuery* query);

std::ostream& operator<<(std::ostream& ostream, const NodeQuery& query);

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_NODE_QUERY_H_
