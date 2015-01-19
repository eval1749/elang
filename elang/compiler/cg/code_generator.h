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
  TypeMapper* type_mapper() const { return type_mapper_.get(); }
  hir::Type* void_type() const { return void_type_; }

  hir::Type* MapType(PredefinedName name) const;
  hir::Type* MapType(ir::Type* type) const;
  void SetOutput(hir::Value* value);
  ir::Node* ValueOf(ast::Node* node) const;

  // ast::Visitor declaration nodes
  void CodeGenerator::VisitMethod(ast::Method* ast_method);

  // ast::Visitor expression nodes
  void CodeGenerator::VisitCall(ast::Call* node);
  void VisitNameReference(ast::NameReference* node);

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node);
  void VisitExpressionStatement(ast::ExpressionStatement* node);

  hir::Editor* editor_;
  hir::Factory* const factory_;
  hir::Function* function_;
  std::unordered_map<ast::Method*, hir::Function*> functions_;
  Output* output_;
  const std::unique_ptr<TypeMapper> type_mapper_;
  hir::Type* const void_type_;

  DISALLOW_COPY_AND_ASSIGN(CodeGenerator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CODE_GENERATOR_H_
