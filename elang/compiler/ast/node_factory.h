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
class QualifiedName;
class Token;

namespace ast {
#define FORWARD_DECLARATION(type) class type;
AST_NODE_LIST(FORWARD_DECLARATION)
#undef FORWARD_DECLARATION

class Expression;
class EnumMember;
class NamespaceBody;

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
                          Token* simple_name,
                          const QualifiedName& target_name);
  public: Class* NewClass(NamespaceBody* namespace_body, Token* keyword,
                          Token* simple_name);
  public: Enum* NewEnum(NamespaceBody* namespace_body, Token* keyword,
                        Token* simple_name);
  public: EnumMember* NewEnumMember(Enum* owner, Token* simple_name,
                                    Expression* expression);
  public: Namespace* NewNamespace(NamespaceBody* namespace_body,
                                  Token* keyword,
                                  Token* simple_name);

  // Expression nodes
  public: Assignment* NewAssignment(Token* op, Expression* left,
                                    Expression* right);
  public: BinaryOperation* NewBinaryOperation(Token* op, Expression* left,
                                              Expression* right);
  public: Conditional* NewConditional(Token* op, Expression* cond_expr,
                                      Expression* then_expr,
                                      Expression* else_expr);
  public: Literal* NewLiteral(Token* literal);
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

