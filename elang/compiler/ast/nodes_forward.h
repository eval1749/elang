// Copyright 2014-2015 Project Vogue. All rights reserved.
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

#define FOR_EACH_DECLARATION_NODE(V) \
  V(Alias)                           \
  V(Import)                          \
  V(Class)                           \
  V(Enum)                            \
  V(Field)                           \
  V(Method)                          \
  V(Namespace)

#define FOR_EACH_EXPRESSION_NODE(V) \
  V(ArrayAccess)                    \
  V(Assignment)                     \
  V(BinaryOperation)                \
  V(Call)                           \
  V(Conditional)                    \
  V(InvalidExpression)              \
  V(Literal)                        \
  V(MemberAccess)                   \
  V(NameReference)                  \
  V(ParameterReference)             \
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

#define FOR_EACH_TYPE_NODE(V) \
  V(ArrayType)                \
  V(ConstructedType)          \
  V(InvalidType)              \
  V(OptionalType)             \
  V(TypeMemberAccess)         \
  V(TypeNameReference)

#define FOR_EACH_CONCRETE_AST_NODE(V) \
  FOR_EACH_DECLARATION_NODE(V)        \
  FOR_EACH_EXPRESSION_NODE(V)         \
  FOR_EACH_STATEMENT_NODE(V)          \
  FOR_EACH_TYPE_NODE(V)               \
  V(CatchClause)                      \
  V(ClassBody)                        \
  V(EnumMember)                       \
  V(Parameter)                        \
  V(Variable)                         \
  V(MethodGroup)                      \
  V(NamespaceBody)

#define FOR_EACH_ABSTRACT_AST_NODE(V) \
  V(BodyNode)                         \
  V(ContainerNode)                    \
  V(Expression)                       \
  V(Node)                             \
  V(NamedNode)                        \
  V(NamespaceNode)                    \
  V(Statement)                        \
  V(Type)

//////////////////////////////////////////////////////////////////////
//
// Forward class declarations
//
#define V(Name) class Name;
FOR_EACH_CONCRETE_AST_NODE(V)
FOR_EACH_ABSTRACT_AST_NODE(V)
#undef V

class Factory;
enum class ParameterKind;
class Visitor;

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODES_FORWARD_H_
