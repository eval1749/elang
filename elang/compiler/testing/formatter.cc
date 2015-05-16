// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/testing/formatter.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// Formatter::FormatBlock
//
Formatter::FormatBlock::FormatBlock(Formatter* formatter,
                                    NewlineAtEnd newline_at_end)
    : formatter_(formatter), newline_at_end_(newline_at_end) {
  formatter_->ostream_ << "{" << std::endl;
  ++formatter_->depth_;
}

Formatter::FormatBlock::~FormatBlock() {
  --formatter_->depth_;
  formatter_->Indent();
  formatter_->ostream_ << "}";
  if (newline_at_end_ == NewlineAtEnd::No)
    return;
  formatter_->ostream_ << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
Formatter::Formatter() : depth_(0) {
}

void Formatter::FormatChildStatement(ast::Statement* statement) {
  if (statement->is<ast::BlockStatement>()) {
    ostream_ << " ";
    Visit(statement);
    return;
  }
  ostream_ << std::endl;
  IndentPlusOne();
  Visit(statement);
}

void Formatter::Indent() {
  ostream_ << std::string(depth_ * 2, ' ');
}

void Formatter::IndentPlusOne() {
  ++depth_;
  Indent();
  --depth_;
}

std::string Formatter::Run(ast::Node* node) {
  ostream_.clear();
  depth_ = 0;
  node->Accept(this);
  return ostream_.str();
}

void Formatter::Visit(ast::Node* node) {
  node->Accept(this);
}

// ast::Visitor
void Formatter::VisitAlias(ast::Alias* alias) {
  Indent();
  ostream_ << alias->keyword() << " " << alias->name() << " = ";
  Visit(alias->reference());
  ostream_ << ";" << std::endl;
}

void Formatter::VisitArrayAccess(ast::ArrayAccess* access) {
  Visit(access->array());
  auto separator = "[";
  for (auto const index : access->indexes()) {
    ostream_ << separator;
    Visit(index);
    separator = ", ";
  }
  ostream_ << "]";
}

void Formatter::VisitArrayType(ast::ArrayType* node) {
  std::vector<ast::ArrayType*> array_types;
  for (ast::Type* runner = node; runner->is<ast::ArrayType>();
       runner = runner->as<ast::ArrayType>()->element_type()) {
    array_types.push_back(runner->as<ast::ArrayType>());
  }
  ostream_ << *array_types.back()->element_type();
  for (auto array_type : array_types) {
    ostream_ << "[";
    auto separator = "";
    for (auto dimension : array_type->dimensions()) {
      ostream_ << separator;
      if (dimension >= 0)
        ostream_ << dimension;
      separator = ",";
    }
    ostream_ << "]";
  }
}

void Formatter::VisitAssignment(ast::Assignment* assignment) {
  Visit(assignment->left());
  ostream_ << " " << assignment->op() << " ";
  Visit(assignment->right());
}

void Formatter::VisitBinaryOperation(ast::BinaryOperation* operation) {
  Visit(operation->left());
  ostream_ << " " << operation->op() << " ";
  Visit(operation->right());
}

void Formatter::VisitBlockStatement(ast::BlockStatement* block_statement) {
  FormatBlock block(this, NewlineAtEnd::No);
  for (auto const statement : block_statement->statements()) {
    Indent();
    Visit(statement);
    ostream_ << std::endl;
  }
}

void Formatter::VisitBreakStatement(ast::BreakStatement* break_statement) {
  __assume(break_statement);
  ostream_ << "break;";
}

void Formatter::VisitCall(ast::Call* call) {
  Visit(call->callee());
  ostream_ << "(";
  auto separator = "";
  for (auto const argument : call->arguments()) {
    ostream_ << separator;
    Visit(argument);
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitCatchClause(ast::CatchClause* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitConditional(ast::Conditional* cond) {
  Visit(cond->condition());
  ostream_ << " ? ";
  Visit(cond->true_expression());
  ostream_ << " : ";
  Visit(cond->false_expression());
}

void Formatter::VisitClass(ast::Class* clazz) {
  DCHECK(clazz);
  NOTREACHED();
}

void Formatter::VisitClassBody(ast::ClassBody* class_body) {
  auto const clazz = class_body->owner();
  Indent();
  ostream_ << clazz->modifiers();
  if (clazz->modifiers().value())
    ostream_ << " ";
  ostream_ << clazz->token() << " " << clazz->name();
  auto separator = " : ";
  for (auto const base_class_name : class_body->base_class_names()) {
    ostream_ << separator;
    Visit(base_class_name);
    separator = ", ";
  }

  ostream_ << " ";
  FormatBlock block(this);
  class_body->AcceptForMembers(this);
}

void Formatter::VisitContinueStatement(
    ast::ContinueStatement* continue_statement) {
  __assume(continue_statement);
  ostream_ << "continue;";
}

void Formatter::VisitConstructedName(ast::ConstructedName* cons_name) {
  Traverse(cons_name->reference());
  ostream_ << "<";
  auto separator = "";
  for (auto const type_arg : cons_name->arguments()) {
    ostream_ << separator;
    Visit(type_arg);
    separator = ", ";
  }
  ostream_ << ">";
}

void Formatter::VisitConstructedType(ast::ConstructedType* cons_type) {
  VisitConstructedName(cons_type->reference());
}

void Formatter::VisitDoStatement(ast::DoStatement* do_statement) {
  ostream_ << "do ";
  Visit(do_statement->statement());
  ostream_ << " while (";
  Visit(do_statement->condition());
  ostream_ << ");";
}

void Formatter::VisitEmptyStatement(ast::EmptyStatement* empty_statement) {
  __assume(empty_statement);
  ostream_ << ";";
}

void Formatter::VisitEnum(ast::Enum* enum_type) {
  Indent();
  ostream_ << "enum " << enum_type->name() << " ";
  if (auto const enum_base = enum_type->enum_base())
    ostream_ << ": " << enum_base << " ";
  FormatBlock block(this);
  for (auto const member : enum_type->members()) {
    member->Accept(this);
    ostream_ << "," << std::endl;
  }
}

void Formatter::VisitEnumMember(ast::EnumMember* node) {
  Indent();
  ostream_ << node->name();
  if (auto const expression = node->expression()) {
    ostream_ << " = ";
    Visit(expression);
  }
}

void Formatter::VisitExpressionList(ast::ExpressionList* statement) {
  auto separator = "";
  for (auto const expression : statement->expressions()) {
    ostream_ << separator;
    Visit(expression);
    separator = ", ";
  }
}

void Formatter::VisitExpressionStatement(ast::ExpressionStatement* statement) {
  Visit(statement->expression());
  ostream_ << ";";
}

void Formatter::VisitField(ast::Field* field) {
  Indent();
  Visit(field->type());
  ostream_ << " " << field->name();
  if (auto const expression = field->expression()) {
    ostream_ << " = ";
    Visit(expression);
  }
  ostream_ << ";" << std::endl;
}

void Formatter::VisitForEachStatement(ast::ForEachStatement* statement) {
  ostream_ << "for (";
  Visit(statement->variable()->type());
  ostream_ << " " << statement->variable()->name() << " : ";
  Visit(statement->enumerable());
  ostream_ << ")";
  FormatChildStatement(statement->statement());
}

void Formatter::VisitForStatement(ast::ForStatement* statement) {
  ostream_ << "for (";
  Visit(statement->initializer());
  if (statement->initializer()->is<ast::ExpressionList>())
    ostream_ << ";";
  if (statement->condition()) {
    ostream_ << " ";
    Visit(statement->condition());
  }
  ostream_ << ";";
  if (statement->step()) {
    ostream_ << " ";
    Visit(statement->step());
  }
  ostream_ << ")";
  FormatChildStatement(statement->statement());
}

void Formatter::VisitIfStatement(ast::IfStatement* statement) {
  ostream_ << "if (";
  Visit(statement->condition());
  ostream_ << ")";
  if (!statement->else_statement()) {
    if (statement->then_statement()->is<ast::BlockStatement>()) {
      ostream_ << " ";
      Visit(statement->then_statement());
      return;
    }
    ostream_ << std::endl;
    IndentPlusOne();
    Visit(statement->then_statement());
    return;
  }

  auto const use_brace =
      statement->then_statement()->is<ast::BlockStatement>() ||
      statement->else_statement()->is<ast::BlockStatement>();

  if (statement->then_statement()->is<ast::BlockStatement>()) {
    ostream_ << " ";
    Visit(statement->then_statement());
  } else if (use_brace) {
    ostream_ << " {" << std::endl;
    IndentPlusOne();
    Visit(statement->then_statement());
    ostream_ << std::endl;
    Indent();
    ostream_ << "}";
  }

  if (statement->else_statement()->is<ast::BlockStatement>()) {
    ostream_ << " else ";
    Visit(statement->else_statement());
    return;
  }

  if (use_brace) {
    ostream_ << " else {" << std::endl;
    IndentPlusOne();
    Visit(statement->else_statement());
    ostream_ << std::endl;
    Indent();
    ostream_ << "}";
    return;
  }

  ostream_ << std::endl;
  Indent();
  ostream_ << "else" << std::endl;
  IndentPlusOne();
  Visit(statement->then_statement());
}

void Formatter::VisitImport(ast::Import* import) {
  Indent();
  ostream_ << import->keyword() << " ";
  Visit(import->reference());
  ostream_ << ";" << std::endl;
}

void Formatter::VisitIncrementExpression(ast::IncrementExpression* node) {
  switch (node->token()->type()) {
    case TokenType::Decrement:
    case TokenType::Increment:
      ostream_ << node->token();
      Visit(node->expression());
      return;
    case TokenType::PostDecrement:
    case TokenType::PostIncrement:
      Visit(node->expression());
      ostream_ << node->token();
      return;
  }
  NOTREACHED() << node->token();
}

void Formatter::VisitInvalidExpression(ast::InvalidExpression* expression) {
  ostream_ << "INVALID('" << expression->token() << "')";
}

void Formatter::VisitInvalidStatement(ast::InvalidStatement* statement) {
  ostream_ << "INVALID '" << statement->token() << "';";
}

void Formatter::VisitInvalidType(ast::InvalidType* type) {
  ostream_ << "INVALID '" << type->token() << "';";
}

void Formatter::VisitLiteral(ast::Literal* operation) {
  ostream_ << operation->token();
}

void Formatter::VisitMemberAccess(ast::MemberAccess* node) {
  Traverse(node->container());
  ostream_ << '.' << node->member();
}

void Formatter::VisitMethod(ast::Method* method) {
  Indent();
  ostream_ << method->modifiers();
  if (method->modifiers().value())
    ostream_ << " ";
  Visit(method->return_type());
  ostream_ << " " << method->name();
  if (!method->type_parameters().empty()) {
    auto separator = "<";
    for (auto const name : method->type_parameters()) {
      ostream_ << separator << name;
      separator = ", ";
    }
    ostream_ << ">";
  }
  ostream_ << "(";
  auto separator = "";
  for (auto const param : method->parameters()) {
    ostream_ << separator;
    Visit(param->type());
    ostream_ << " " << param->name();
    separator = ", ";
  }
  ostream_ << ")";
  auto const statement = method->body();
  if (!statement) {
    ostream_ << ";" << std::endl;
    return;
  }
  if (statement->is<ast::BlockStatement>()) {
    ostream_ << " ";
    Visit(statement);
    ostream_ << std::endl;
    return;
  }
  ostream_ << "=> ";
  Visit(statement);
  ostream_ << ";" << std::endl;
}

void Formatter::VisitMethodBody(ast::MethodBody* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitMethodGroup(ast::MethodGroup* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitNameReference(ast::NameReference* operation) {
  ostream_ << operation->token();
}

void Formatter::VisitNamespace(ast::Namespace* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitNamespaceBody(ast::NamespaceBody* ns_body) {
  // TODO(eval1749) We should have specific way to detect loaded classes.
  if (ns_body->loaded_) {
    for (auto const member : ns_body->members()) {
      auto const node = member->as<ast::NamespaceBody>();
      if (!node)
        continue;
      VisitNamespaceBody(node);
    }
    return;
  }
  if (!ns_body->owner()->parent()) {
    // We don't print "namespace" text for global namespace.
    ns_body->AcceptForMembers(this);
    return;
  }
  Indent();
  ostream_ << "namespace " << ns_body->name() << " ";
  FormatBlock block(this);
  ns_body->AcceptForMembers(this);
}

void Formatter::VisitOptionalType(ast::OptionalType* type) {
  Visit(type->base_type());
  ostream_ << "?";
}

void Formatter::VisitParameter(ast::Parameter* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitParameterReference(ast::ParameterReference* param) {
  ostream_ << param->token();
}

void Formatter::VisitReturnStatement(ast::ReturnStatement* return_statement) {
  ostream_ << "return";
  if (auto const value = return_statement->value()) {
    ostream_ << " ";
    Visit(value);
  }
  ostream_ << ";";
}

void Formatter::VisitThrowStatement(ast::ThrowStatement* throw_statement) {
  ostream_ << "throw";
  if (auto const value = throw_statement->value()) {
    ostream_ << " ";
    Visit(value);
  }
  ostream_ << ";";
}

void Formatter::VisitTryStatement(ast::TryStatement* try_statement) {
  ostream_ << "try ";
  Visit(try_statement->protected_block());
  for (auto const catch_clause : try_statement->catch_clauses()) {
    ostream_ << " catch (";
    Visit(catch_clause->type());
    if (auto const variable = catch_clause->variable())
      ostream_ << " " << variable->name();
    ostream_ << ") ";
    Visit(catch_clause->block());
  }
  auto const finally_block = try_statement->finally_block();
  if (!finally_block)
    return;
  ostream_ << " finally ";
  Visit(finally_block);
}

void Formatter::VisitTypeMemberAccess(ast::TypeMemberAccess* type) {
  Visit(type->reference());
}

void Formatter::VisitTypeNameReference(ast::TypeNameReference* type) {
  Visit(type->reference());
}

void Formatter::VisitUnaryOperation(ast::UnaryOperation* operation) {
  if (operation->op() == TokenType::PostDecrement ||
      operation->op() == TokenType::PostIncrement) {
    Visit(operation->expression());
    ostream_ << operation->op();
    return;
  }
  ostream_ << operation->op();
  Visit(operation->expression());
}

void Formatter::VisitUsingStatement(ast::UsingStatement* using_statement) {
  ostream_ << "using (";
  if (auto const var = using_statement->variable()) {
    ostream_ << "var ";
    ostream_ << var->name();
    ostream_ << " = ";
  }
  Visit(using_statement->resource());
  ostream_ << ")";
  if (using_statement->statement()->is<ast::BlockStatement>()) {
    ostream_ << " ";
    Visit(using_statement->statement());
  } else {
    ostream_ << std::endl;
    Indent();
    Visit(using_statement->statement());
  }
}

void Formatter::VisitVarDeclaration(ast::VarDeclaration* node) {
  ostream_ << node->name() << " = ";
  Visit(node->value());
}

void Formatter::VisitVarStatement(ast::VarStatement* var_statement) {
  if (var_statement->keyword() == TokenType::Const)
    ostream_ << "const ";
  auto is_first = true;
  for (auto const variable : var_statement->variables()) {
    if (is_first) {
      Visit(variable->type());
      ostream_ << " ";
      is_first = false;
    } else {
      ostream_ << ", ";
    }
    Visit(variable);
  }
  ostream_ << ";";
}

void Formatter::VisitVariable(ast::Variable* node) {
  DCHECK(node);
  NOTREACHED();
}

void Formatter::VisitVariableReference(ast::VariableReference* var) {
  ostream_ << var->token();
}

void Formatter::VisitWhileStatement(ast::WhileStatement* while_statement) {
  ostream_ << "while (";
  Visit(while_statement->condition());
  ostream_ << ") ";
  Visit(while_statement->statement());
}

void Formatter::VisitYieldStatement(ast::YieldStatement* yield_statement) {
  ostream_ << "yield ";
  Visit(yield_statement->value());
  ostream_ << ";";
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
