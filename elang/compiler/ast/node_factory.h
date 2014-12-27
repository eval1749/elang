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
  private: std::vector<std::unique_ptr<Node>> nodes_;

  public: NodeFactory();
  public: ~NodeFactory();

  // Declaration related nodes
  public: Alias* NewAlias(NamespaceBody* namespace_body, Token* keyword,
                          Token* alias_name,
                          const QualifiedName& target_name);
  public: Class* NewClass(NamespaceBody* namespace_body, Modifiers modifiers,
                          Token* keyword, Token* name);
  public: Enum* NewEnum(NamespaceBody* namespace_body, Modifiers modifiers,
                        Token* keyword, Token* name);
  public: EnumMember* NewEnumMember(Enum* owner, Token* name,
                                    Expression* expression);
  public: Field* NewField(NamespaceBody* namespace_body,  Modifiers modifiers,
                          Expression* type, Token* name,
                          Expression* expression);
  public: Namespace* NewNamespace(NamespaceBody* namespace_body,
                                  Token* keyword, Token* name);

  // Expression nodes
  public: ArrayType* NewArrayType(
      Token* op,
      Expression* element_type,
      const std::vector<int>& ranks);
  public: Assignment* NewAssignment(Token* op, Expression* left,
                                    Expression* right);
  public: BinaryOperation* NewBinaryOperation(Token* op, Expression* left,
                                              Expression* right);
  public: Conditional* NewConditional(Token* op, Expression* cond_expr,
                                      Expression* then_expr,
                                      Expression* else_expr);
  public: ConstructedType* NewConstructedType(
      Token* op,
      Expression* blueprint_type,
      const std::vector<Expression*>& arguments);
  public: Literal* NewLiteral(Token* literal);
  public: MemberAccess* NewMemberAccess(
      const std::vector<Expression*>& members);
  public: NameReference* NewNameReference(Token* literal);
  public: UnaryOperation* NewUnaryOperation(Token* op, Expression* expr);

  // Utility
  private: Node* RememberNode(Node* node);
  public: void RemoveAll();

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_node_factory_h)

