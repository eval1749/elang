// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>

#include "elang/compiler/translate/translator.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/temporary_change_value.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"
#include "elang/compiler/translate/type_mapper.h"
#include "elang/optimizer/editor.h"
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
      editor_(nullptr),
      output_(nullptr),
      type_mapper_(new IrTypeMapper(session, factory->type_factory())) {
}

Translator::~Translator() {
}

void Translator::Commit() {
  editor()->Commit();
}

void Translator::EmitOutput(ir::Node* node) {
  DCHECK(!output_);
  output_ = node;
}

ir::Type* Translator::MapType(PredefinedName name) const {
  return type_mapper_->Map(name);
}

ir::Type* Translator::MapType(sm::Type* type) const {
  return type_mapper_->Map(type);
}

// The entry point of |Translator::|.
bool Translator::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

ir::Node* Translator::Translate(ast::Node* node) {
  DCHECK(!output_);
  node->Accept(this);
  DCHECK(output_);
  return output_;
}

ir::Node* Translator::TranslateLiteral(ir::Type* type, const Token* token) {
  if (type == MapType(PredefinedName::Bool))
    return NewBool(token->bool_data());

  if (type == MapType(PredefinedName::Char))
    return NewChar(token->char_data());

  if (type == MapType(PredefinedName::Float32))
    return NewFloat32(token->f32_data());

  if (type == MapType(PredefinedName::Float64))
    return NewFloat64(token->f64_data());

  if (type == MapType(PredefinedName::Int16))
    return NewInt16(static_cast<int16_t>(token->int64_data()));

  if (type == MapType(PredefinedName::Int32))
    return NewInt32(static_cast<int32_t>(token->int64_data()));

  if (type == MapType(PredefinedName::Int64))
    return NewInt64(token->int64_data());

  if (type == MapType(PredefinedName::Int8))
    return NewInt8(static_cast<int8_t>(token->int64_data()));

  if (type == MapType(PredefinedName::UInt16))
    return NewUInt16(static_cast<uint16_t>(token->int64_data()));

  if (type == MapType(PredefinedName::UInt32))
    return NewUInt32(static_cast<uint32_t>(token->int64_data()));

  if (type == MapType(PredefinedName::UInt64))
    return NewUInt64(static_cast<uint64_t>(token->int64_data()));

  if (type == MapType(PredefinedName::UInt8))
    return NewUInt8(static_cast<uint8_t>(token->int64_data()));

  NOTREACHED() << "Bad literal token " << *token;
  return nullptr;
}

sm::Semantic* Translator::ValueOf(ast::Node* node) const {
  return semantics()->ValueOf(node);
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
  DCHECK(!editor_);
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
      type_mapper()->Map(method->signature())->as<ir::FunctionType>());
  session()->RegisterFunction(ast_method, function);

  ir::Editor editor(factory(), function);
  TemporaryChangeValue<ir::Editor*> editor_scope(editor_, &editor);
  editor.Edit(function->entry_node());

  // TODO(eval1749) emit parameter bindings
  // TODO(eval1749) handle body expression
  Translate(ast_method_body);
  if (!editor.control())
    return;
  if (MapType(method->return_type()) != MapType(PredefinedName::Void) &&
      function->exit_node()->input(0)->CountInputs()) {
    Error(ErrorCode::TranslatorReturnNone, ast_method);
  }
  editor.Commit();
  DCHECK(editor.Validate()) << editor.errors();
}

//
// ast::Visitor expression nodes
//

void Translator::VisitLiteral(ast::Literal* node) {
  auto const value = ValueOf(node)->as<sm::Literal>();
  EmitOutput(TranslateLiteral(MapType(value->type()), node->token()));
}

//
// ast::Visitor statement nodes
//
void Translator::VisitBlockStatement(ast::BlockStatement* node) {
  for (auto const statement : node->statements()) {
    if (!editor()->control()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
    Translate(statement);
  }
}

void Translator::VisitExpressionList(ast::ExpressionList* node) {
  for (auto const expression : node->expressions())
    expression->Accept(this);
}

void Translator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  DCHECK(!output_);
  node->expression()->Accept(this);
}

void Translator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const return_value =
      node->value() ? Translate(node->value()) : void_value();
  editor()->SetRet(return_value);
  Commit();
}

}  // namespace compiler
}  // namespace elang
