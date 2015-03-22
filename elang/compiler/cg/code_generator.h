// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CODE_GENERATOR_H_
#define ELANG_COMPILER_CG_CODE_GENERATOR_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"
#include "elang/compiler/ir/nodes_forward.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {
class Editor;
class TypeFactory;
}
namespace compiler {

class TypeMapper;
class VariableAnalyzer;

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
class CodeGenerator final : public CompilationSessionUser, public ast::Visitor {
 public:
  CodeGenerator(CompilationSession* session,
                hir::Factory* factory,
                VariableAnalyzer* variable_analyzer);
  ~CodeGenerator();

  bool Run();

 private:
  struct Output;
  class ScopedOutput;
  struct BreakContext;
  class ScopedBreakContext;

  hir::Type* bool_type() const;
  hir::Editor* editor() const { return editor_; }
  hir::Factory* factory() const { return factory_; }
  hir::TypeFactory* types() const;
  TypeMapper* type_mapper() const { return type_mapper_.get(); }
  hir::Type* void_type() const { return void_type_; }
  hir::Value* void_value() const;

  hir::Type* MapType(PredefinedName name) const;
  hir::Type* MapType(sm::Type* type) const;

  // Returns new instruction yields value of |type| with |left| and |right|
  // inputs for |node|
  hir::Instruction* NewInstructionFor(ast::Expression* node,
                                      hir::Type* type,
                                      hir::Value* left,
                                      hir::Value* right);

  // Generate value
  void Generate(ast::Statement* statement);
  hir::Value* GenerateArrayAccess(ast::ArrayAccess* expression);
  hir::Value* GenerateBool(ast::Expression* expression);
  void GenerateDoOrWhile(ast::DoOrWhileStatement* do_or_while_statement);
  hir::Value* GenerateValue(ast::Expression* expression);
  hir::Value* GenerateValueAs(ast::Expression* expression, hir::Type* type);
  hir::Value* NewLiteral(hir::Type* type, const Token* token);
  hir::Value* NewMethodReference(sm::Method* method);

  // Output
  void Commit();
  void Emit(hir::Instruction* instruction);
  hir::BasicBlock* EmitMergeBlock();
  void EmitOutput(hir::Value* value);
  void EmitOutputInstruction(hir::Instruction* instruction);
  void EmitParameterBindings(ast::Method* method);
  void EmitVariableAssignment(ast::NamedNode* ast_node,
                              ast::Expression* ast_value);
  void EmitVariableBinding(ast::NamedNode* ast_node, hir::Value* value);
  void EmitVariableReference(ast::NamedNode* node);
  bool NeedOutput() const;

  // Shortcut for |semantics()->ValueOf()|
  sm::Node* ValueOf(ast::Node* node) const;

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;

  // ast::Visitor declaration nodes
  void CodeGenerator::VisitMethod(ast::Method* ast_method) final;

  // ast::Visitor expression nodes
  void VisitArrayAccess(ast::ArrayAccess* node) final;
  void VisitAssignment(ast::Assignment* node) final;
  void VisitBinaryOperation(ast::BinaryOperation* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitConditional(ast::Conditional* node) final;
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

  const BreakContext* break_context_;
  hir::Editor* editor_;
  hir::Factory* const factory_;
  hir::Function* function_;
  Output* output_;
  const std::unique_ptr<TypeMapper> type_mapper_;
  VariableAnalyzer* const variable_analyzer_;
  hir::Type* const void_type_;

  // Map |sm::Variable| to value or pointer producing instruction.
  std::unordered_map<sm::Variable*, hir::Value*> variables_;

  DISALLOW_COPY_AND_ASSIGN(CodeGenerator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CODE_GENERATOR_H_
