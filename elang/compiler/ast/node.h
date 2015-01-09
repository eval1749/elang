// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODE_H_
#define ELANG_COMPILER_AST_NODE_H_

#include <memory>

#include "elang/base/castable.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

#define FOR_EACH_DECLARATION_NODE(V) \
  V(Alias)                           \
  V(Class)                           \
  V(Enum)                            \
  V(Field)                           \
  V(Import)                          \
  V(Method)                          \
  V(MethodGroup)                     \
  V(Namespace)

#define FOR_EACH_EXPRESSION_NODE(V) \
  V(ArrayAccess)                    \
  V(ArrayType)                      \
  V(Assignment)                     \
  V(BinaryOperation)                \
  V(Call)                           \
  V(Conditional)                    \
  V(ConstructedType)                \
  V(InvalidExpression)              \
  V(Literal)                        \
  V(MemberAccess)                   \
  V(NameReference)                  \
  V(UnaryOperation)                 \
  V(VariableReference)

#define FOR_EACH_STATEMENT_NODE(V) \
  V(BlockStatement)                \
  V(BreakStatement)                \
  V(DoStatement)                   \
  V(ContinueStatement)             \
  V(EmptyStatement)                \
  V(ExpressionStatement)           \
  V(ExpressionList)                \
  V(ForEachStatement)              \
  V(ForStatement)                  \
  V(IfStatement)                   \
  V(InvalidStatement)              \
  V(ReturnStatement)               \
  V(ThrowStatement)                \
  V(TryStatement)                  \
  V(UsingStatement)                \
  V(VarStatement)                  \
  V(WhileStatement)                \
  V(YieldStatement)

#define FOR_EACH_AST_NODE(V)   \
  FOR_EACH_DECLARATION_NODE(V) \
  FOR_EACH_EXPRESSION_NODE(V)  \
  FOR_EACH_STATEMENT_NODE(V)

//////////////////////////////////////////////////////////////////////
//
// Forward class declarations
//
#define FORWARD_DECLARATION(type) class type;
FOR_EACH_AST_NODE(FORWARD_DECLARATION)
#undef FORWARD_DECLARATION

class CatchClause;
class NamedNode;
class EnumMember;
class Expression;
class LocalVariable;
class Method;
class MemberContainer;
class NamespaceBody;
class NodeFactory;
class Statement;
class Visitor;

#define DECLARE_AST_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class NodeFactory;                 \
                                            \
 protected:                                 \
  ~self() = default;

#define DECLARE_AST_NODE_CONCRETE_CLASS(self, super) \
  DECLARE_AST_NODE_CLASS(self, super);               \
  void Accept(Visitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public Visitable<Visitor>, public ZoneAllocated {
  DECLARE_AST_NODE_CLASS(Node, Castable);

 public:
  Token* token() const { return token_; }

  // Visitable<Visitor>
  // Default implementation for node classes not in |FOR_EACH_AST_NODE()|.
  void Accept(Visitor* visitor) override;

 protected:
  explicit Node(Token* token);

 private:
  Token* const token_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

//////////////////////////////////////////////////////////////////////
//
// NamedNode
//
class NamedNode : public Node {
  DECLARE_AST_NODE_CLASS(NamedNode, Node);

 public:
  Token* keyword() const { return token(); }
  Token* name() const { return name_; }

 protected:
  NamedNode(Token* keyword, Token* name);

 private:
  Token* const name_;

  DISALLOW_COPY_AND_ASSIGN(NamedNode);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODE_H_
