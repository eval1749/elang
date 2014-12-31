// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODE_H_
#define ELANG_COMPILER_AST_NODE_H_

#include <memory>

#include "elang/base/castable.h"
#include "elang/base/types.h"
#include "elang/base/zone_object.h"
#include "elang/compiler/token.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {
namespace ast {

#define DECLARATION_NODE_LIST(V) \
  V(Alias)                       \
  V(Class)                       \
  V(Enum)                        \
  V(Field)                       \
  V(Import)                      \
  V(MethodGroup)                 \
  V(Namespace)

#define EXPRESSION_NODE_LIST(V) \
  V(ArrayType)                  \
  V(Assignment)                 \
  V(BinaryOperation)            \
  V(Call)                       \
  V(Conditional)                \
  V(ConstructedType)            \
  V(Literal)                    \
  V(MemberAccess)               \
  V(NameReference)              \
  V(UnaryOperation)

#define STATEMENT_NODE_LIST(V) \
  V(BlockStatement)            \
  V(BreakStatement)            \
  V(DoStatement)               \
  V(ContinueStatement)         \
  V(EmptyStatement)            \
  V(ExpressionStatement)       \
  V(IfStatement)               \
  V(ReturnStatement)           \
  V(ThrowStatement)            \
  V(TryStatement)              \
  V(UsingStatement)            \
  V(VarStatement)              \
  V(WhileStatement)            \
  V(YieldStatement)

#define AST_NODE_LIST(V)   \
  DECLARATION_NODE_LIST(V) \
  EXPRESSION_NODE_LIST(V)  \
  STATEMENT_NODE_LIST(V)

//////////////////////////////////////////////////////////////////////
//
// Forward class declarations
//
#define FORWARD_DECLARATION(type) class type;
AST_NODE_LIST(FORWARD_DECLARATION)
#undef FORWARD_DECLARATION

class CatchClause;
class NamedNode;
class EnumMember;
class Expression;
class LocalVariable;
class Method;
class NamespaceBody;
class NodeFactory;
class Statement;
class Visitor;

#define DECLARE_AST_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);      \
  friend class NodeFactory;                 \
                                            \
 protected:                                 \
  ~self() = default;                        \
                                            \
 private:
//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public ZoneObject {
  DECLARE_AST_NODE_CLASS(Node, Castable);

 public:
  Token* token() const { return token_; }

  virtual void Accept(Visitor* visitor);

 protected:
  explicit Node(Token* token);

 private:
  Token* const token_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODE_H_
