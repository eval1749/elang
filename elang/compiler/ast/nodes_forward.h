// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODES_FORWARD_H_
#define ELANG_COMPILER_AST_NODES_FORWARD_H_

#include <memory>

#include "elang/base/castable.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

// Note: |Method| is derived from |NamedNode| rather than |NamespaceMember|.
// |MethodGroup| contains |Method| instances.
#define FOR_EACH_DECLARATION_NODE(V) \
  V(Alias)                           \
  V(Import)                          \
  V(Class)                           \
  V(Enum)                            \
  V(Field)                           \
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
#define V(Name) class Name;
FOR_EACH_AST_NODE(V)
#undef V

class CatchClause;
class NamedNode;
class EnumMember;
class Expression;
class LocalVariable;
class Method;
class MemberContainer;
class NamespaceBody;
class Node;
class NodeFactory;
class Statement;
class Visitor;

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODES_FORWARD_H_
