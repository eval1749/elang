// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_FACTORY_H_
#define ELANG_COMPILER_AST_FACTORY_H_

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/zone_user.h"
#include "elang/base/float_types.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {
class CompilationSession;
class Modifiers;
class Token;

namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public ZoneUser {
 public:
  explicit Factory(CompilationSession* session);
  ~Factory();

  // Declaration related nodes
  Alias* NewAlias(NamespaceBody* namespace_body,
                  Token* keyword,
                  Token* alias_name,
                  Expression* reference);
  ClassBody* NewClassBody(BodyNode* outer,
                          Modifiers modifiers,
                          Token* keyword,
                          Token* name,
                          const std::vector<Type*>& base_class_names);
  Const* NewConst(ClassBody* class_body,
                  Modifiers modifiers,
                  Token* keyword,
                  Type* type,
                  Token* name,
                  Expression* expression);
  Enum* NewEnum(BodyNode* outer,
                Modifiers modifiers,
                Token* keyword,
                Token* name,
                Type* enum_base);
  EnumMember* NewEnumMember(Enum* owner,
                            Token* name,
                            Expression* explicit_expression,
                            Expression* implicit_expression);
  Field* NewField(ClassBody* class_body,
                  Modifiers modifiers,
                  Token* keyword,
                  Type* type,
                  Token* name,
                  Expression* expression);
  Import* NewImport(NamespaceBody* namespace_body,
                    Token* keyword,
                    Expression* reference);
  Method* NewMethod(ClassBody* outer,
                    Modifiers modifies,
                    Type* type,
                    Token* name,
                    const std::vector<Token*>& type_parameters);
  MethodBody* NewMethodBody(Method* method);
  Namespace* NewNamespace(Namespace* outer, Token* keyword, Token* name);
  NamespaceBody* NewNamespaceBody(NamespaceBody* outer,
                                  Token* keyword,
                                  Token* name);

  // Expression nodes
  ArrayAccess* NewArrayAccess(Token* bracket,
                              Expression* array,
                              const std::vector<Expression*> indexes);
  Assignment* NewAssignment(Token* op, Expression* left, Expression* right);
  BinaryOperation* NewBinaryOperation(Token* op,
                                      Expression* left,
                                      Expression* right);
  Call* NewCall(Expression* callee, const std::vector<Expression*> arguments);
  Conditional* NewConditional(Token* op,
                              Expression* cond_expr,
                              Expression* then_expr,
                              Expression* else_expr);
  ConstructedName* NewConstructedName(Expression* reference,
                                      const std::vector<Type*>& arguments);
  IncrementExpression* NewIncrementExpression(Token* op, Expression* expr);
  InvalidExpression* NewInvalidExpression(Token* token);
  Literal* NewLiteral(Token* literal);
  MemberAccess* NewMemberAccess(Expression* container, Token* member);
  NameReference* NewNameReference(Token* name);
  Expression* NewNoExpression(Token* token);
  ParameterReference* NewParameterReference(Token* name, Parameter* param);
  UnaryOperation* NewUnaryOperation(Token* op, Expression* expr);
  VariableReference* NewVariableReference(Token* name, Variable* var);

  // Statement nodes
  BlockStatement* NewBlockStatement(Token* keyword,
                                    const std::vector<Statement*> statements);
  BreakStatement* NewBreakStatement(Token* keyword);
  CatchClause* NewCatchClause(Token* keyword,
                              Type* type,
                              Variable* variable,
                              BlockStatement* block);
  ContinueStatement* NewContinueStatement(Token* keyword);
  DoStatement* NewDoStatement(Token* keyword,
                              Statement* statement,
                              Expression* condition);
  EmptyStatement* NewEmptyStatement(Token* keyword);
  ExpressionStatement* NewExpressionStatement(Expression* expression);

  ExpressionList* NewExpressionList(
      Token* keyword,
      const std::vector<Expression*>& expressions);

  ForEachStatement* NewForEachStatement(Token* keyword,
                                        Variable* variable,
                                        Expression* enumerable,
                                        Statement* statement);

  ForStatement* NewForStatement(Token* keyword,
                                Statement* initializer,
                                Expression* condition,
                                Statement* step,
                                Statement* statement);

  IfStatement* NewIfStatement(Token* keyword,
                              Expression* condition,
                              Statement* then_statement,
                              Statement* else_statement);
  InvalidStatement* NewInvalidStatement(Token* token);
  ReturnStatement* NewReturnStatement(Token* keyword, Expression* value);
  ThrowStatement* NewThrowStatement(Token* keyword, Expression* value);
  TryStatement* NewTryStatement(Token* keyword,
                                BlockStatement* protected_block,
                                const std::vector<CatchClause*>& catch_clauses,
                                BlockStatement* finally_block);
  UsingStatement* NewUsingStatement(Token* keyword,
                                    Variable* variable,
                                    Expression* resource,
                                    Statement* statement);
  VarDeclaration* NewVarDeclaration(Token* token,
                                    Variable* variable,
                                    Expression* expression);
  VarStatement* NewVarStatement(Token* keyword,
                                const std::vector<VarDeclaration*>& variables);
  WhileStatement* NewWhileStatement(Token* keyword,
                                    Expression* condition,
                                    Statement* statement);
  YieldStatement* NewYieldStatement(Token* keyword, Expression* value);

  // Type nodes
  ArrayType* NewArrayType(Token* op,
                          Type* element_type,
                          const std::vector<int>& dimensions);
  ConstructedType* NewConstructedType(ConstructedName* node);
  InvalidType* NewInvalidType(Expression* expression);
  OptionalType* NewOptionalType(Token* op, Type* base_type);
  TypeMemberAccess* NewTypeMemberAccess(NamespaceBody* namespace_body,
                                        MemberAccess* node);
  TypeMemberAccess* NewTypeMemberAccess(MemberAccess* node);
  TypeNameReference* NewTypeNameReference(NamespaceBody* namespace_body,
                                          NameReference* node);
  TypeNameReference* NewTypeNameReference(NameReference* node);
  TypeVariable* NewTypeVariable(Token* token);

  // Misc nodes
  Parameter* NewParameter(Method* owner,
                          ParameterKind kind,
                          int position,
                          Type* type,
                          Token* name,
                          Expression* expression);
  Variable* NewVariable(Token* keyword, Type* type, Token* name);

 private:
  Node* RememberNode(Node* node);
  void SetParent(Node* child, Node* parent);

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_FACTORY_H_
