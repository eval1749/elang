// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/translate/translator.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"
#include "elang/compiler/translate/builder.h"
#include "elang/compiler/translate/type_mapper.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Translator
//
Translator::Translator(CompilationSession* session, ir::Factory* factory)
    : CompilationSessionUser(session),
      FactoryUser(factory),
      break_context_(nullptr),
      builder_(nullptr),
      type_mapper_(new IrTypeMapper(session, factory->type_factory())),
      visit_result_(nullptr) {
}

Translator::~Translator() {
}

void Translator::BindParameters(ast::Method* method) {
  if (method->parameters().empty())
    return;
  auto index = 0;
  for (auto const parameter : method->parameters()) {
    auto const variable = ValueOf(parameter)->as<sm::Variable>();
    DCHECK(variable);
    builder_->BindVariable(variable, builder()->ParameterAt(index));
    ++index;
  }
}

void Translator::SetVisitorResult(ir::Node* node) {
  DCHECK(!visit_result_);
  visit_result_ = node;
}

ir::Type* Translator::MapType(PredefinedName name) const {
  return type_mapper_->Map(name);
}

ir::Type* Translator::MapType(sm::Type* type) const {
  return type_mapper_->Map(type);
}

// The entry point of |Translator::|.
void Translator::Run() {
  session()->Apply(this);
}

sm::Semantic* Translator::ValueOf(ast::Node* node) const {
  return analysis()->SemanticOf(node);
}

//
// ast::Visitor
//
void Translator::DoDefaultVisit(ast::Node* node) {
  if (node->is<ast::Expression>()) {
    Error(ErrorCode::TranslatorExpressionNotYetImplemented, node);
    return;
  }
  if (node->is<ast::Statement>()) {
    Error(ErrorCode::TranslatorStatementNotYetImplemented, node);
    return;
  }
  ast::Visitor::DoDefaultVisit(node);
}

//
// ast::Visitor declaration nodes
//

void Translator::VisitMethod(ast::Method* ast_method) {
  DCHECK(!builder_);
  //  1 Convert ast::FunctionType to ir::FunctionType
  //  2 ir::NewFunction(function_type)
  auto const method = ValueOf(ast_method)->as<sm::Method>();
  if (!method) {
    DVLOG(0) << "Not resolved " << *ast_method;
    return;
  }
  auto const ast_method_body = ast_method->body();
  if (!ast_method_body)
    return;
  auto const function = factory()->NewFunction(
      type_mapper()->Map(method->function_signature())->as<ir::FunctionType>());
  session()->RegisterFunction(ast_method, function);

  Builder builder(factory(), function);
  base::AutoReset<Builder*> builder_scope(&builder_, &builder);

  BindParameters(ast_method);
  // TODO(eval1749) handle body expression
  TranslateStatement(ast_method_body->as<ast::Statement>());
  if (!builder_->has_control())
    return;
  if (method->return_type() != PredefinedTypeOf(PredefinedName::Void) &&
      function->exit_node()->input(0)->CountInputs()) {
    Error(ErrorCode::TranslatorReturnNone, ast_method);
  }
  builder_->EndBlockWithRet(void_value());
}

}  // namespace compiler
}  // namespace elang
