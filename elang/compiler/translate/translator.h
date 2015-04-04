// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_
#define ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "base/macros.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"
#include "elang/compiler/semantics/nodes_forward.h"
#include "elang/optimizer/factory_user.h"
#include "elang/optimizer/nodes_forward.h"

namespace elang {
namespace optimizer {
class TypeFactory;
}
namespace compiler {

namespace ir = optimizer;

class Builder;
class IrTypeMapper;

//////////////////////////////////////////////////////////////////////
//
// |Translator| translates an AST function to an IR function.
//
class Translator final : public CompilationSessionUser,
                         public ast::Visitor,
                         public ir::FactoryUser {
 public:
  Translator(CompilationSession* session, ir::Factory* factory);
  ~Translator();

  bool Run();

 private:
  struct BreakContext;
  class ScopedBreakContext;

  Builder* builder() const { return builder_; }
  IrTypeMapper* type_mapper() const { return type_mapper_.get(); }

  // Builder
  void Commit();

  // Type management
  ir::Type* MapType(PredefinedName name) const;
  ir::Type* MapType(sm::Type* type) const;

  // Translate
  ir::Data* NewOperationFor(ast::Expression* node,
                            ir::Data* left,
                            ir::Data* right);
  void SetVisitorResult(ir::Node* node);

  ir::Data* TranslateAs(ast::Expression* expression, ir::Type* ir_type);
  ir::Data* TranslateBool(ast::Expression* expression);
  ir::Data* Translate(ast::Expression* node);
  ir::Data* TranslateLiteral(ir::Type* ir_type, const Token* token);
  ir::Data* TranslateMethodReference(sm::Method* method);
  void TranslateStatement(ast::Statement* node);
  void TranslateVariable(ast::NamedNode* ast_variable);
  void TranslateVariableAssignment(ast::NamedNode* ast_variable,
                                   ast::Expression* ast_value);

  // Shortcut for |semantics()->ValueOf()|
  sm::Semantic* ValueOf(ast::Node* node) const;

  // Variable management
  void BindParameters(ast::Method* method);

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;

  // ast::Visitor declaration nodes
  void VisitMethod(ast::Method* ast_method) final;

  // ast::Visitor expression nodes
  void VisitAssignment(ast::Assignment* node) final;
  void VisitBinaryOperation(ast::BinaryOperation* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitLiteral(ast::Literal* node) final;
  void VisitParameterReference(ast::ParameterReference* node) final;
  void VisitVariableReference(ast::VariableReference* node) final;

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitBreakStatement(ast::BreakStatement* node) final;
  void VisitContinueStatement(ast::ContinueStatement* node) final;
  void VisitDoStatement(ast::DoStatement* node) final;
  void VisitExpressionList(ast::ExpressionList* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitForEachStatement(ast::ForEachStatement* node) final;
  void VisitIfStatement(ast::IfStatement* node) final;
  void VisitReturnStatement(ast::ReturnStatement* node) final;
  void VisitVarStatement(ast::VarStatement* node) final;

  // Holds current |break| and |continue| targets.
  const BreakContext* break_context_;

  // The |Builder|.
  Builder* builder_;

  // The type mapper for mapping |sm::Type| to |ir::Type|.
  const std::unique_ptr<IrTypeMapper> type_mapper_;

  // List of variables bound in current block scope.
  std::vector<sm::Variable*> variables_;

  // Holds the result of visiting.
  ir::Node* visit_result_;

  DISALLOW_COPY_AND_ASSIGN(Translator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_
