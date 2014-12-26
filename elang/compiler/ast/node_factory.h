// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_node_factory_h)
#define INCLUDE_elang_compiler_ast_node_factory_h

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {
class QualifiedName;
class Token;

namespace ast {
class Alias;
class Class;
class Enum;
class EnumMember;
class Expression;
class Namespace;
class NamespaceBody;
class Node;

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
class NodeFactory final {
  private: std::vector<std::unique_ptr<Node>> nodes_;

  public: NodeFactory();
  public: ~NodeFactory();

  public: Alias* NewAlias(NamespaceBody* namespace_body, Token* keyword,
                          Token* simple_name,
                          const QualifiedName& target_name);
  public: Class* NewClass(NamespaceBody* namespace_body, Token* keyword,
                          Token* simple_name);
  public: Enum* NewEnum(NamespaceBody* namespace_body, Token* keyword,
                        Token* simple_name);
  public: EnumMember* NewEnumMember(Enum* owner, Token* simple_name,
                                    Expression* expression);

  public: Expression* NewExpression(Token* operator_token,
                                    std::vector<Expression*> operands);
  public: Expression* NewExpression(Token* operator_token,
                                    Expression* operand0,
                                    Expression* operand1);
  public: Expression* NewExpression(Token* operator_token,
                                    Expression* operand0);
  public: Expression* NewExpression(Token* operator_token);
  public: Namespace* NewNamespace(NamespaceBody* namespace_body,
                                  Token* keyword,
                                  Token* simple_name);
  public: void RemoveAll();

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_node_factory_h)

