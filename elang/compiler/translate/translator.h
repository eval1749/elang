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

  void Run();

 private:
  struct BreakContext;
  struct Reference;
  class ScopedBreakContext;

  Builder* builder() const { return builder_; }
  IrTypeMapper* type_mapper() const { return type_mapper_.get(); }

  // Builder
  void Commit();

  // Type management
  ir::Type* MapType(PredefinedName name) const;
  ir::Type* MapType(sm::Type* type) const;

  // Translate
  ir::Node* NewDataOrTuple(const std::vector<ir::Node*> nodes);
  ir::Data* NewOperationFor(ast::Expression* node,
                            ir::Data* left,
                            ir::Data* right);
  // Shortcut for |analysis()->SemanticOf()|
  sm::Semantic* SemanticOf(ast::Node* node) const;
  void SetVisitorResult(ir::Node* node);

  ir::Data* TranslateAs(ast::Expression* expression, ir::Type* ir_type);
  ir::Data* TranslateBool(ast::Expression* expression);
  ir::Data* Translate(ast::Expression* node);
  Reference TranslateElement(ast::ArrayAccess* node);
  Reference TranslateField(sm::Field* node);
  ir::Data* TranslateLiteral(ir::Type* ir_type, const Token* token);
  ir::Data* TranslateMethodReference(sm::Method* method);
  void TranslateStatement(ast::Statement* node);
  void TranslateVariable(ast::NamedNode* ast_variable);
  void TranslateVariableAssignment(ast::NamedNode* ast_variable,
                                   ast::Expression* ast_value);

  // Variable management
  void BindParameters(ast::Method* method);

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;

  // ast::Visitor declaration nodes
  void VisitAlias(ast::Alias* node) final;
  void VisitImport(ast::Import* node) final;
  void VisitMethod(ast::Method* node) final;

  // ast::Visitor expression nodes
  void VisitArrayAccess(ast::ArrayAccess* node) final;
  void VisitAssignment(ast::Assignment* node) final;
  void VisitBinaryOperation(ast::BinaryOperation* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitLiteral(ast::Literal* node) final;
  void VisitNameReference(ast::NameReference* node) final;
  void VisitParameterReference(ast::ParameterReference* node) final;
  void VisitVariableReference(ast::VariableReference* node) final;

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitBreakStatement(ast::BreakStatement* node) final;
  void VisitContinueStatement(ast::ContinueStatement* node) final;
  void VisitDoStatement(ast::DoStatement* node) final;
  void VisitExpressionList(ast::ExpressionList* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitForStatement(ast::ForStatement* node) final;
  void VisitForEachStatement(ast::ForEachStatement* node) final;
  void VisitIfStatement(ast::IfStatement* node) final;
  void VisitReturnStatement(ast::ReturnStatement* node) final;
  void VisitVarStatement(ast::VarStatement* node) final;
  void VisitWhileStatement(ast::WhileStatement* node) final;

  // Holds current |break| and |continue| targets.
  const BreakContext* break_context_;

  // The |Builder|.
  Builder* builder_;

  // A method being translated.
  sm::Method* method_;

  // The type mapper for mapping |sm::Type| to |ir::Type|.
  const std::unique_ptr<IrTypeMapper> type_mapper_;

  // Holds the result of visiting.
  ir::Node* visit_result_;

  DISALLOW_COPY_AND_ASSIGN(Translator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_
