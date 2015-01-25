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

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
class CodeGenerator final : public CompilationSessionUser, public ast::Visitor {
 public:
  CodeGenerator(CompilationSession* session, hir::Factory* factory);
  ~CodeGenerator();

  hir::Function* FunctionOf(ast::Method* method) const;
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
  hir::Type* MapType(ir::Type* type) const;

  // Generate value
  void Generate(ast::Statement* statement);
  hir::Value* GenerateBool(ast::Expression* expression);
  hir::Value* GenerateValue(ast::Expression* expression);
  hir::Value* NewLiteral(hir::Type* type, const Token* token);

  // Output
  void Commit();
  void Emit(hir::Instruction* instruction);
  void EmitOutput(hir::Value* value);
  void EmitOutputInstruction(hir::Instruction* instruction);
  void EmitParameterBindings(ast::Method* method);
  void EmitVariableAssignment(ast::NamedNode* ast_node,
                              ast::Expression* ast_value);
  void EmitVariableBinding(ast::NamedNode* ast_node,
                           ast::Expression* ast_value,
                           hir::Value* value);
  void EmitVariableReference(ast::NamedNode* node);
  bool NeedOutput() const;

  // Shortcut for |semantics()->ValueOf()|
  ir::Node* ValueOf(ast::Node* node) const;

  // ast::Visitor declaration nodes
  void CodeGenerator::VisitMethod(ast::Method* ast_method);

  // ast::Visitor expression nodes
  void VisitAssignment(ast::Assignment* node);
  void VisitCall(ast::Call* node);
  void VisitConditional(ast::Conditional* node);
  void VisitLiteral(ast::Literal* node);
  void VisitNameReference(ast::NameReference* node);
  void VisitParameterReference(ast::ParameterReference* node);
  void VisitVariableReference(ast::VariableReference* node);

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitBreakStatement(ast::BreakStatement* node);
  void VisitContinueStatement(ast::ContinueStatement* node);
  void VisitDoStatement(ast::DoStatement* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);
  void VisitIfStatement(ast::IfStatement* node);
  void VisitReturnStatement(ast::ReturnStatement* node);
  void VisitVarStatement(ast::VarStatement* node);

  const BreakContext* break_context_;
  hir::Editor* editor_;
  hir::Factory* const factory_;
  hir::Function* function_;
  std::unordered_map<ast::Method*, hir::Function*> functions_;
  Output* output_;
  const std::unique_ptr<TypeMapper> type_mapper_;
  hir::Type* const void_type_;

  // Map |ir::Variable| to value or pointer producing instruction.
  std::unordered_map<ir::Variable*, hir::Value*> variables_;

  DISALLOW_COPY_AND_ASSIGN(CodeGenerator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CODE_GENERATOR_H_
