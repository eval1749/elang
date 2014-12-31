// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NODE_FACTORY_H_
#define ELANG_COMPILER_AST_NODE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/ast/node.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
class Zone;
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
  explicit NodeFactory(Zone* zone);
  ~NodeFactory();

  // Declaration related nodes
  Alias* NewAlias(NamespaceBody* namespace_body,
                  Token* keyword,
                  Token* alias_name,
                  Expression* reference);
  Class* NewClass(NamespaceBody* namespace_body,
                  Modifiers modifiers,
                  Token* keyword,
                  Token* name);
  Enum* NewEnum(NamespaceBody* namespace_body,
                Modifiers modifiers,
                Token* keyword,
                Token* name);
  EnumMember* NewEnumMember(Enum* owner, Token* name, Expression* expression);
  Field* NewField(NamespaceBody* namespace_body,
                  Modifiers modifiers,
                  Expression* type,
                  Token* name,
                  Expression* expression);
  Import* NewImport(NamespaceBody* namespace_body,
                    Token* keyword,
                    Expression* reference);
  Method* NewMethod(NamespaceBody* namespace_body,
                    MethodGroup* method_group,
                    Modifiers modifies,
                    Expression* type,
                    Token* name,
                    const std::vector<Token*>& type_parameters,
                    const std::vector<LocalVariable*>& parameters);
  MethodGroup* NewMethodGroup(NamespaceBody* namespace_body, Token* name);
  Namespace* NewNamespace(NamespaceBody* namespace_body,
                          Token* keyword,
                          Token* name);
  NamespaceBody* NewNamespaceBody(NamespaceBody* outer, Namespace* owner);

  // Expression nodes
  ArrayType* NewArrayType(Token* op,
                          Expression* element_type,
                          const std::vector<int>& ranks);
  Assignment* NewAssignment(Token* op, Expression* left, Expression* right);
  BinaryOperation* NewBinaryOperation(Token* op,
                                      Expression* left,
                                      Expression* right);
  Call* NewCall(Expression* callee, const std::vector<Expression*> arguments);
  Conditional* NewConditional(Token* op,
                              Expression* cond_expr,
                              Expression* then_expr,
                              Expression* else_expr);
  ConstructedType* NewConstructedType(
      Expression* blueprint_type,
      const std::vector<Expression*>& arguments);
  Literal* NewLiteral(Token* literal);
  MemberAccess* NewMemberAccess(Token* name,
                                const std::vector<Expression*>& members);
  NameReference* NewNameReference(Token* literal);
  UnaryOperation* NewUnaryOperation(Token* op, Expression* expr);

  // Statement nodes
  BlockStatement* NewBlockStatement(Token* keyword,
                                    const std::vector<Statement*> statements);
  BreakStatement* NewBreakStatement(Token* keyword);
  CatchClause* NewCatchClause(Token* keyword,
                              Expression* type,
                              LocalVariable* variable,
                              BlockStatement* block);
  ContinueStatement* NewContinueStatement(Token* keyword);
  DoStatement* NewDoStatement(Token* keyword,
                              Statement* statement,
                              Expression* condition);
  EmptyStatement* NewEmptyStatement(Token* keyword);
  ExpressionStatement* NewExpressionStatement(Expression* expression);
  IfStatement* NewIfStatement(Token* keyword,
                              Expression* condition,
                              Statement* then_statement,
                              Statement* else_statement);
  LocalVariable* NewLocalVariable(Token* keyword,
                                  Expression* type,
                                  Token* name,
                                  Expression* expression);
  ReturnStatement* NewReturnStatement(Token* keyword, Expression* value);
  ThrowStatement* NewThrowStatement(Token* keyword, Expression* value);
  TryStatement* NewTryStatement(Token* keyword,
                                BlockStatement* protected_block,
                                const std::vector<CatchClause*>& catch_clauses,
                                BlockStatement* finally_block);
  UsingStatement* NewUsingStatement(Token* keyword,
                                    LocalVariable* variable,
                                    Expression* resource,
                                    Statement* statement);
  VarStatement* NewVarStatement(Token* keyword,
                                const std::vector<LocalVariable*>& variables);
  WhileStatement* NewWhileStatement(Token* keyword,
                                    Expression* condition,
                                    Statement* statement);
  YieldStatement* NewYieldStatement(Token* keyword, Expression* value);

 private:
  Node* RememberNode(Node* node);

  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NODE_FACTORY_H_
