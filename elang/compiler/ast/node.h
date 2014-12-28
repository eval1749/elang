// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_node_h)
#define INCLUDE_elang_compiler_ast_node_h

#include <memory>

#include "elang/base/castable.h"
#include "elang/base/types.h"
#include "elang/compiler/token.h"

namespace elang {
namespace compiler {
namespace ast {

#define DECLARATION_NODE_LIST(V) \
  V(Alias) \
  V(Class) \
  V(Enum) \
  V(Field) \
  V(Method) \
  V(Namespace)

#define EXPRESSION_NODE_LIST(V) \
  V(ArrayType) \
  V(Assignment) \
  V(BinaryOperation) \
  V(Conditional) \
  V(ConstructedType) \
  V(Literal) \
  V(MemberAccess) \
  V(NameReference) \
  V(UnaryOperation)

#define AST_NODE_LIST(V) \
  DECLARATION_NODE_LIST(V) \
  EXPRESSION_NODE_LIST(V)

//////////////////////////////////////////////////////////////////////
//
// Forward class declarations
//
#define FORWARD_DECLARATION(type) class type;
AST_NODE_LIST(FORWARD_DECLARATION)
#undef FORWARD_DECLARATION

class EnumMember;
class Expression;
class NamespaceBody;
class Visitor;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable {
  DECLARE_CASTABLE_CLASS(Node, Castable);

 public:
  virtual ~Node();

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

#endif  // !defined(INCLUDE_elang_compiler_ast_node_h)
