// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_
#define ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_

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

class IrTypeMapper;

//////////////////////////////////////////////////////////////////////
//
// Translator
//
class Translator final : public CompilationSessionUser,
                         public ast::Visitor,
                         public ir::FactoryUser {
 public:
  Translator(CompilationSession* session, ir::Factory* factory);
  ~Translator();

  bool Run();

 private:
  ir::Editor* editor() const { return editor_; }
  IrTypeMapper* type_mapper() const { return type_mapper_.get(); }

  void Commit();
  void EmitOutput(ir::Node* node);

  ir::Type* MapType(PredefinedName name) const;
  ir::Type* MapType(sm::Type* type) const;

  ir::Node* Translate(ast::Node* node);
  ir::Node* TranslateLiteral(ir::Type* ir_type, const Token* token);

  // Shortcut for |semantics()->ValueOf()|
  sm::Semantic* ValueOf(ast::Node* node) const;

  // ast::Visitor
  void DoDefaultVisit(ast::Node* node) final;

  // ast::Visitor declaration nodes
  void VisitMethod(ast::Method* ast_method) final;

  // ast::Visitor expression nodes
  void VisitLiteral(ast::Literal* node) final;

  // ast::Visitor statement nodes
  void VisitBlockStatement(ast::BlockStatement* node) final;
  void VisitExpressionList(ast::ExpressionList* node) final;
  void VisitExpressionStatement(ast::ExpressionStatement* node) final;
  void VisitReturnStatement(ast::ReturnStatement* node) final;

  ir::Editor* editor_;
  ir::Node* output_;
  const std::unique_ptr<IrTypeMapper> type_mapper_;

  DISALLOW_COPY_AND_ASSIGN(Translator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TRANSLATOR_H_
