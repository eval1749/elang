// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_node_factory_h)
#define INCLUDE_elang_compiler_ast_node_factory_h

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/ast/node.h"
#include "elang/compiler/ast/type_and_name.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {
class Modifiers;
class QualifiedName;
class Token;

namespace ast {

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
class NodeFactory final {
 public:
  NodeFactory();
  ~NodeFactory();

  // Declaration related nodes
  Alias* NewAlias(NamespaceBody* namespace_body, Token* keyword,
                  Token* alias_name, const QualifiedName& target_name);
  Class* NewClass(NamespaceBody* namespace_body, Modifiers modifiers,
                  Token* keyword, Token* name);
  Enum* NewEnum(NamespaceBody* namespace_body, Modifiers modifiers,
                Token* keyword, Token* name);
  EnumMember* NewEnumMember(Enum* owner, Token* name, Expression* expression);
  Field* NewField(NamespaceBody* namespace_body,  Modifiers modifiers,
                  Expression* type, Token* name, Expression* expression);
  Method* NewMethod(NamespaceBody* namespace_body, Modifiers modifies,
                    Expression* type, Token* name,
                    const std::vector<Token*>& type_parameters,
                    const std::vector<TypeAndName>& parameters);
  Namespace* NewNamespace(NamespaceBody* namespace_body, Token* keyword,
                          Token* name);

  // Expression nodes
  ArrayType* NewArrayType(Token* op, Expression* element_type,
                          const std::vector<int>& ranks);
  Assignment* NewAssignment(Token* op, Expression* left, Expression* right);
  BinaryOperation* NewBinaryOperation(Token* op, Expression* left,
                                      Expression* right);
  Conditional* NewConditional(Token* op, Expression* cond_expr,
                              Expression* then_expr, Expression* else_expr);
  ConstructedType* NewConstructedType(
    Token* op, Expression* blueprint_type,
    const std::vector<Expression*>& arguments);
  Literal* NewLiteral(Token* literal);
  MemberAccess* NewMemberAccess(const std::vector<Expression*>& members);
  NameReference* NewNameReference(Token* literal);
  UnaryOperation* NewUnaryOperation(Token* op, Expression* expr);

  // Utility
  void RemoveAll();

 private:
  Node* RememberNode(Node* node);

  std::vector<std::unique_ptr<Node>> nodes_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_node_factory_h)

