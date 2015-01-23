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

  hir::Factory* factory() const { return factory_; }
  hir::TypeFactory* types() const;
  TypeMapper* type_mapper() const { return type_mapper_.get(); }
  hir::Type* void_type() const { return void_type_; }

  hir::Type* MapType(PredefinedName name) const;
  hir::Type* MapType(ir::Type* type) const;

  // Generate value
  hir::Value* GenerateValue(hir::Type* type, ast::Expression* expression);
  hir::Value* NewLiteral(hir::Type* type, const Token* token);

  // Output
  void Emit(hir::Instruction* instruction);
  void EmitParameterBindings(ast::Method* method);
  void EmitVariableBinding(ast::NamedNode* ast_node,
                           ast::Expression* ast_value,
                           hir::Value* value);
  void EmitVariableReference(ast::NamedNode* node);
  bool NeedOutput() const;
  void SetOutput(hir::Instruction* instruction);
  void SetOutput(hir::Value* value);

  // Shortcut for |semantics()->ValueOf()|
  ir::Node* ValueOf(ast::Node* node) const;

  // ast::Visitor declaration nodes
  void CodeGenerator::VisitMethod(ast::Method* ast_method);

  // ast::Visitor expression nodes
  void CodeGenerator::VisitCall(ast::Call* node);
  void VisitLiteral(ast::Literal* node);
  void VisitNameReference(ast::NameReference* node);
  void VisitParameterReference(ast::ParameterReference* node);
  void VisitVariableReference(ast::VariableReference* node);

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);
  void VisitReturnStatement(ast::ReturnStatement* node);
  void VisitVarStatement(ast::VarStatement* node);

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
