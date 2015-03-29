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

namespace {
bool IsUnsignedType(ir::Node* node) {
  auto const type = node->output_type()->as<ir::PrimitiveValueType>();
  return type && type->is_unsigned();
}
}

//////////////////////////////////////////////////////////////////////
//
// Translator::BreakContext represents target blocks of |break| and
// |continue| statements. |switch| statement also specify |continue_block|
// from outer |BreakContext|.
//
struct Translator::BreakContext final {
  ir::Node* break_block;
  ir::Node* continue_block;
  const BreakContext* outer;

  BreakContext(const BreakContext* outer,
               ir::Node* break_block,
               ir::Node* continue_block)
      : break_block(break_block),
        continue_block(continue_block),
        outer(outer) {}
};

class Translator::ScopedBreakContext final {
 public:
  ScopedBreakContext(Translator* generator,
                     ir::Node* break_block,
                     ir::Node* continue_block);
  ~ScopedBreakContext();

 private:
  Translator* const translator_;
  BreakContext const context_;

  DISALLOW_COPY_AND_ASSIGN(ScopedBreakContext);
};

Translator::ScopedBreakContext::ScopedBreakContext(Translator* generator,
                                                   ir::Node* break_block,
                                                   ir::Node* continue_block)
    : translator_(generator),
      context_(translator_->break_context_, break_block, continue_block) {
  translator_->break_context_ = &context_;
}

Translator::ScopedBreakContext::~ScopedBreakContext() {
  DCHECK_EQ(translator_->break_context_, &context_);
  translator_->break_context_ = context_.outer;
}

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

ir::Node* Translator::NewOperationFor(ast::Expression* node,
                                      ir::Node* left,
                                      ir::Node* right) {
  if (left->output_type()->is_float()) {
    switch (node->op()->type()) {
      case TokenType::Add:
        return NewFloatAdd(left, right);
      case TokenType::Div:
        return NewFloatDiv(left, right);
      case TokenType::Eq:
        return NewFloatCmp(ir::FloatCondition::OrderedEqual, left, right);
      case TokenType::Ge:
        return NewFloatCmp(ir::FloatCondition::OrderedGreaterThanOrEqual, left,
                           right);
      case TokenType::Gt:
        return NewFloatCmp(ir::FloatCondition::OrderedGreaterThan, left, right);
      case TokenType::Le:
        return NewFloatCmp(ir::FloatCondition::OrderedLessThan, left, right);
      case TokenType::Lt:
        return NewFloatCmp(ir::FloatCondition::UnorderedLessThanOrEqual, left,
                           right);
      case TokenType::Mul:
        return NewFloatMul(left, right);
      case TokenType::Ne:
        return NewFloatCmp(ir::FloatCondition::OrderedNotEqual, left, right);
      case TokenType::Sub:
        return NewFloatSub(left, right);
    }
    NOTREACHED() << "Unexpected AST node: " << *node;
    Error(ErrorCode::TranslatorExpressionUnexpected, node);
    return void_value();
  }
  if (left->output_type()->is_integer()) {
    switch (node->op()->type()) {
      case TokenType::Add:
        return NewIntAdd(left, right);
      case TokenType::BitAnd:
        return NewIntBitAnd(left, right);
      case TokenType::BitOr:
        return NewIntBitOr(left, right);
      case TokenType::BitXor:
        return NewIntBitXor(left, right);
      case TokenType::Div:
        return NewIntDiv(left, right);
      case TokenType::Eq:
        return NewIntCmp(ir::IntCondition::Equal, left, right);
      case TokenType::Ge:
        if (IsUnsignedType(left)) {
          return NewIntCmp(ir::IntCondition::UnsignedGreaterThanOrEqual, left,
                           right);
        }
        return NewIntCmp(ir::IntCondition::SignedGreaterThanOrEqual, left,
                         right);
      case TokenType::Gt:
        if (IsUnsignedType(left))
          return NewIntCmp(ir::IntCondition::UnsignedGreaterThan, left, right);
        return NewIntCmp(ir::IntCondition::SignedGreaterThan, left, right);
      case TokenType::Le:
        if (IsUnsignedType(left))
          return NewIntCmp(ir::IntCondition::UnsignedLessThan, left, right);
        return NewIntCmp(ir::IntCondition::SignedLessThan, left, right);
      case TokenType::Lt:
        if (IsUnsignedType(left)) {
          return NewIntCmp(ir::IntCondition::UnsignedLessThanOrEqual, left,
                           right);
        }
        return NewIntCmp(ir::IntCondition::SignedLessThanOrEqual, left, right);
      case TokenType::Mul:
        return NewIntMul(left, right);
      case TokenType::Ne:
        return NewIntCmp(ir::IntCondition::NotEqual, left, right);
      case TokenType::Shl:
        return NewIntShl(left, right);
      case TokenType::Shr:
        return NewIntShr(left, right);
      case TokenType::Sub:
        return NewIntSub(left, right);
    }
  }
  switch (node->op()->type()) {
    case TokenType::Eq:
      return NewIntCmp(ir::IntCondition::Equal, left, right);
    case TokenType::Ne:
      return NewIntCmp(ir::IntCondition::NotEqual, left, right);
  }
  NOTREACHED() << "Unexpected AST node: " << *node;
  Error(ErrorCode::TranslatorExpressionUnexpected, node);
  return void_value();
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

// Translate |expression| to produce |type|.
ir::Node* Translator::TranslateAs(ast::Expression* expression, ir::Type* type) {
  auto const node = Translate(expression);
  if (node->output_type() == type)
    return node;
  return NewStaticCast(type, node);
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
    return NewInt16(token->int16_data());

  if (type == MapType(PredefinedName::Int32))
    return NewInt32(token->int32_data());

  if (type == MapType(PredefinedName::Int64))
    return NewInt64(token->int64_data());

  if (type == MapType(PredefinedName::Int8))
    return NewInt8(token->int8_data());

  if (type == MapType(PredefinedName::UInt16))
    return NewUInt16(token->uint16_data());

  if (type == MapType(PredefinedName::UInt32))
    return NewUInt32(token->uint32_data());

  if (type == MapType(PredefinedName::UInt64))
    return NewUInt64(token->uint64_data());

  if (type == MapType(PredefinedName::UInt8))
    return NewUInt8(token->int8_data());

  NOTREACHED() << "Bad literal token " << *token;
  return void_value();
}

void Translator::TranslateStatement(ast::Statement* node) {
  DCHECK(!visit_result_);
  node->Accept(this);
  DCHECK(!visit_result_);
}

void Translator::TranslateVariable(ast::NamedNode* ast_variable) {
  auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
  DCHECK(variable);
  SetVisitorResult(builder_->VariableValueOf(variable));
}

void Translator::TranslateVariableAssignment(ast::NamedNode* ast_variable,
                                             ast::Expression* ast_value) {
  auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
  auto const value = Translate(ast_value);
  builder_->AssignVariable(variable, value);
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
      type_mapper()->Map(method->signature())->as<ir::FunctionType>());
  session()->RegisterFunction(ast_method, function);

  auto const builder = std::make_unique<Builder>(factory(), function);
  TemporaryChangeValue<Builder*> builder_scope(builder_, builder.get());

  BindParameters(ast_method);
  // TODO(eval1749) handle body expression
  TranslateStatement(ast_method_body->as<ast::Statement>());
  if (!builder_->control())
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

void Translator::VisitBinaryOperation(ast::BinaryOperation* node) {
  if (node->is_conditional()) {
    // TODO(eval1749) NYI conditional operation
    DoDefaultVisit(node);
  }
  auto const sm_type = ValueOf(node)->as<sm::Class>();
  DCHECK(sm_type) << "NYI user defined operator: " << *node;
  auto const type = MapType(sm_type);
  auto const lhs = TranslateAs(node->left(), type);
  auto const rhs = TranslateAs(node->right(), type);
  SetVisitorResult(NewOperationFor(node, lhs, rhs));
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
    if (!builder()->control()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
    TranslateStatement(statement);
  }

  // Unbind variables declared in this block.
  if (builder_->control()) {
    for (auto const variable : variables_)
      builder_->UnbindVariable(variable);
  }
  variables_.clear();

  // Restore list of binding variables in this block.
  variables_.swap(variables);
}

void Translator::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  builder_->EndBlockWithJump(break_context_->break_block);
}

void Translator::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  DCHECK(break_context_->continue_block);
  builder_->EndBlockWithJump(break_context_->continue_block);
}

void Translator::VisitDoStatement(ast::DoStatement* node) {
  auto const break_block = NewMerge({});
  auto const continue_block = NewMerge({});

  auto const loop_block = builder_->StartLoopBlock();
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    TranslateStatement(node->statement());
  }
  builder_->EndBlockWithJump(continue_block);

  builder_->StartMergeBlock(continue_block);
  auto const condition = TranslateBool(node->condition());
  builder_->EndLoopBlock(condition, loop_block, break_block);

  builder_->StartMergeBlock(break_block);
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
  auto const if_node = builder_->EndBlockWithBranch(condition);
  auto const merge_node = NewMerge({});

  builder_->StartIfBlock(NewIfTrue(if_node));
  TranslateStatement(node->then_statement());
  builder_->EndBlockWithJump(merge_node);

  builder_->StartIfBlock(NewIfFalse(if_node));
  if (node->else_statement())
    TranslateStatement(node->else_statement());
  builder_->EndBlockWithJump(merge_node);

  if (!merge_node->CountInputs())
    return;
  builder_->StartMergeBlock(merge_node);
}

void Translator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const value = node->value();
  builder()->EndBlockWithRet(value ? Translate(value) : void_value());
}

void Translator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const ast_variable : node->variables()) {
    auto const variable = ValueOf(ast_variable)->as<sm::Variable>();
    DCHECK(variable);
    builder_->BindVariable(variable, Translate(ast_variable->value()));
    variables_.push_back(variable);
  }
}

}  // namespace compiler
}  // namespace elang
