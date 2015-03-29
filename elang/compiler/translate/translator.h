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
class Editor;
class TypeFactory;
}
namespace compiler {

namespace ir = optimizer;

class Editor;
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
  Editor* editor() const { return editor_; }
  IrTypeMapper* type_mapper() const { return type_mapper_.get(); }

  // Editor
  void Commit();

  // Type management
  ir::Type* MapType(PredefinedName name) const;
  ir::Type* MapType(sm::Type* type) const;

  // Translate
  void SetVisitorResult(ir::Node* node);
  ir::Node* Translate(ast::Expression* node);
  ir::Node* TranslateLiteral(ir::Type* ir_type, const Token* token);
  void TranslateStatement(ast::Statement* node);
  ir::Node* TranslateBool(ast::Expression* expression);
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
  void VisitLiteral(ast::Literal* node) final;
  void VisitParameterReference(ast::ParameterReference* node) final;
  void VisitVariableReference(ast::VariableReference* node) final;

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitExpressionList(ast::ExpressionList* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitIfStatement(ast::IfStatement* node) final;
  void VisitReturnStatement(ast::ReturnStatement* node) final;
  void VisitVarStatement(ast::VarStatement* node) final;

  // The |Editor|.
  Editor* editor_;

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
