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
#include "elang/compiler/translate/editor.h"
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
    editor_->BindVariable(variable, editor()->ParameterAt(index));
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
bool Translator::Run() {
  VisitNamespaceBody(session()->global_namespace_body());
  return session()->errors().empty();
}

ir::Node* Translator::Translate(ast::Expression* node) {
  DCHECK(!visit_result_);
  node->Accept(this);
  DCHECK(visit_result_) << *node;
  auto const result = visit_result_;
  visit_result_ = nullptr;
  return result;
}

ir::Node* Translator::TranslateBool(ast::Expression* expression) {
  // TOOD(eval1749) Convert |condition| to |bool|
  auto const node = Translate(expression);
  DCHECK_EQ(node->output_type(), bool_type()) << *node;
  return node;
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

void Translator::TranslateStatement(ast::Statement* node) {
  DCHECK(!visit_result_);
  node->Accept(this);
  DCHECK(!visit_result_);
}

void Translator::TranslateVariable(ast::NamedNode* ast_variable) {
  auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
  DCHECK(variable);
  SetVisitorResult(editor_->VariableValueOf(variable));
}

void Translator::TranslateVariableAssignment(ast::NamedNode* ast_variable,
                                             ast::Expression* ast_value) {
  auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
  auto const value = Translate(ast_value);
  editor_->AssignVariable(variable, value);
  SetVisitorResult(value);
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

  auto const editor = std::make_unique<Editor>(factory(), function);
  TemporaryChangeValue<Editor*> editor_scope(editor_, editor.get());

  BindParameters(ast_method);
  // TODO(eval1749) handle body expression
  TranslateStatement(ast_method_body->as<ast::Statement>());
  if (!editor_->control())
    return;
  if (MapType(method->return_type()) != MapType(PredefinedName::Void) &&
      function->exit_node()->input(0)->CountInputs()) {
    Error(ErrorCode::TranslatorReturnNone, ast_method);
  }
}

//
// ast::Visitor expression nodes
//
// There are five patterns:
//  1. parameter = expression
//  2. variable = expression
//  3. array[index+] = expression
//  5. name = expression; field or property assignment
//  4. container.member = expression; member assignment
void Translator::VisitAssignment(ast::Assignment* node) {
  auto const lhs = node->left();
  auto const rhs = node->right();
  if (auto const reference = lhs->as<ast::ParameterReference>()) {
    TranslateVariableAssignment(reference->parameter(), rhs);
    return;
  }
  if (auto const reference = lhs->as<ast::VariableReference>()) {
    TranslateVariableAssignment(reference->variable(), rhs);
    return;
  }
  Error(ErrorCode::TranslatorExpressionNotYetImplemented, node);
}

void Translator::VisitLiteral(ast::Literal* node) {
  auto const value = ValueOf(node)->as<sm::Literal>();
  SetVisitorResult(TranslateLiteral(MapType(value->type()), node->token()));
}

void Translator::VisitParameterReference(ast::ParameterReference* node) {
  TranslateVariable(node->parameter());
}

void Translator::VisitVariableReference(ast::VariableReference* node) {
  TranslateVariable(node->variable());
}

//
// ast::Visitor statement nodes
//
void Translator::VisitBlockStatement(ast::BlockStatement* node) {
  // Save list of variable bindings.
  std::vector<sm::Variable*> variables;
  variables.swap(variables_);

  for (auto const statement : node->statements()) {
    if (!editor()->control()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
    TranslateStatement(statement);
  }

  // Unbind variables declared in this block.
  if (editor_->control()) {
    for (auto const variable : variables_)
      editor_->UnbindVariable(variable);
  }
  variables_.clear();

  // Restore list of binding variables in this block.
  variables_.swap(variables);
}

void Translator::VisitExpressionList(ast::ExpressionList* node) {
  for (auto const expression : node->expressions())
    Translate(expression);
}

void Translator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  Translate(node->expression());
}

void Translator::VisitIfStatement(ast::IfStatement* node) {
  auto const condition = TranslateBool(node->condition());
  auto const if_node = editor_->EndBlockWithBranch(condition);
  auto const merge_node = NewMerge({});

  editor_->StartIfBlock(NewIfTrue(if_node));
  TranslateStatement(node->then_statement());
  editor_->EndBlockWithJump(merge_node);

  editor_->StartIfBlock(NewIfFalse(if_node));
  if (node->else_statement())
    TranslateStatement(node->else_statement());
  editor_->EndBlockWithJump(merge_node);

  if (!merge_node->CountInputs())
    return;
  editor_->StartMergeBlock(merge_node);
}

void Translator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const value = node->value();
  editor()->EndBlockWithRet(value ? Translate(value) : void_value());
}

void Translator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const ast_variable : node->variables()) {
    auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
    DCHECK(variable);
    editor_->BindVariable(variable, Translate(ast_variable->value()));
    variables_.push_back(variable);
  }
}

}  // namespace compiler
}  // namespace elang
