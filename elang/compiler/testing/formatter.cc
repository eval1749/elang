// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/formatter.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/import.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

std::ostream& operator<<(std::ostream& ostream, const QualifiedName& name) {
  const char* dot = "";
  for (const auto& simple_name : name.simple_names()) {
    ostream << dot << simple_name;
    dot = ".";
  }
  return ostream;
}

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// Formatter::FormatBlock
//
Formatter::FormatBlock::FormatBlock(Formatter* formatter,
                                    NewlineAtEnd newline_at_end)
    : formatter_(formatter), newline_at_end_(newline_at_end) {
  formatter_->stream_ << "{" << std::endl;
  ++formatter_->depth_;
}

Formatter::FormatBlock::~FormatBlock() {
  --formatter_->depth_;
  formatter_->Indent();
  formatter_->stream_ << "}";
  if (newline_at_end_ == NewlineAtEnd::No)
    return;
  formatter_->stream_ << std::endl;
}

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
Formatter::Formatter() : depth_(0) {
}

void Formatter::FormatChildStatement(ast::Statement* statement) {
  if (statement->is<ast::BlockStatement>()) {
    stream_ << " ";
    Visit(statement);
    return;
  }
  stream_ << std::endl;
  IndentPlusOne();
  Visit(statement);
}

void Formatter::Indent() {
  stream_ << std::string(depth_ * 2, ' ');
}

void Formatter::IndentPlusOne() {
  ++depth_;
  Indent();
  --depth_;
}

std::string Formatter::Run(ast::Namespace* ns) {
  stream_.clear();
  depth_ = 0;
  for (auto const namespace_body : ns->bodies()) {
    for (auto const member : namespace_body->members())
      Visit(member);
  }
  return stream_.str();
}

// ast::Vistior
void Formatter::Visit(ast::Node* node) {
  node->Accept(this);
}

void Formatter::VisitAlias(ast::Alias* alias) {
  Indent();
  stream_ << alias->keyword() << " " << alias->name() << " = ";
  Visit(alias->reference());
  stream_ << ";" << std::endl;
}

void Formatter::VisitArrayAccess(ast::ArrayAccess* access) {
  Visit(access->array());
  const char* separator = "[";
  for (auto const index : access->indexes()) {
    stream_ << separator;
    Visit(index);
    separator = ", ";
  }
  stream_ << "]";
}

void Formatter::VisitArrayType(ast::ArrayType* array_type) {
  Visit(array_type->element_type());
  for (auto const rank : array_type->ranks()) {
    stream_ << "[";
    for (auto counter = rank; counter >= 2; --counter)
      stream_ << ",";
    stream_ << "]";
  }
}

void Formatter::VisitAssignment(ast::Assignment* assignment) {
  Visit(assignment->left());
  stream_ << " " << assignment->op() << " ";
  Visit(assignment->right());
}

void Formatter::VisitBinaryOperation(ast::BinaryOperation* operation) {
  Visit(operation->left());
  stream_ << " " << operation->op() << " ";
  Visit(operation->right());
}

void Formatter::VisitBlockStatement(ast::BlockStatement* block_statement) {
  FormatBlock block(this, NewlineAtEnd::No);
  for (auto const statement : block_statement->statements()) {
    Indent();
    Visit(statement);
    stream_ << std::endl;
  }
}

void Formatter::VisitBreakStatement(ast::BreakStatement* break_statement) {
  __assume(break_statement);
  stream_ << "break;";
}

void Formatter::VisitCall(ast::Call* call) {
  Visit(call->callee());
  stream_ << "(";
  const char* separator = "";
  for (auto const argument : call->arguments()) {
    stream_ << separator;
    Visit(argument);
    separator = ", ";
  }
  stream_ << ")";
}

void Formatter::VisitConditional(ast::Conditional* cond) {
  Visit(cond->conditional());
  stream_ << " ? ";
  Visit(cond->then_expression());
  stream_ << " : ";
  Visit(cond->else_expression());
}

void Formatter::VisitClass(ast::Class* klass) {
  for (auto const body : klass->bodies()) {
    Indent();
    stream_ << klass->token() << " " << klass->name();
    const char* separator = " : ";
    for (auto const base_class_name : klass->base_class_names()) {
      stream_ << separator;
      Visit(base_class_name);
      separator = ", ";
    }

    stream_ << " ";
    FormatBlock block(this);
    for (auto const member : body->members())
      Visit(member);
  }
}

void Formatter::VisitConstructedType(ast::ConstructedType* cons_type) {
  Visit(cons_type->blueprint_type());
  stream_ << "<";
  const char* separator = "";
  for (auto const type_arg : cons_type->arguments()) {
    stream_ << separator;
    Visit(type_arg);
    separator = ", ";
  }
  stream_ << ">";
}

void Formatter::VisitContinueStatement(
    ast::ContinueStatement* continue_statement) {
  __assume(continue_statement);
  stream_ << "continue;";
}

void Formatter::VisitDoStatement(ast::DoStatement* do_statement) {
  stream_ << "do ";
  Visit(do_statement->statement());
  stream_ << " while (";
  Visit(do_statement->condition());
  stream_ << ");";
}

void Formatter::VisitEmptyStatement(ast::EmptyStatement* empty_statement) {
  __assume(empty_statement);
  stream_ << ";";
}

void Formatter::VisitEnum(ast::Enum* enumx) {
  Indent();
  stream_ << "enum " << enumx->name() << " ";
  FormatBlock block(this);
  for (auto const member : enumx->members()) {
    Indent();
    stream_ << member->name();
    if (auto const expression = member->expression()) {
      stream_ << " = ";
      Visit(expression);
    }
    stream_ << "," << std::endl;
  }
}

void Formatter::VisitExpressionList(ast::ExpressionList* statement) {
  const char* separator = "";
  for (auto const expression : statement->expressions()) {
    stream_ << separator;
    Visit(expression);
    separator = ", ";
  }
}

void Formatter::VisitExpressionStatement(ast::ExpressionStatement* statement) {
  Visit(statement->expression());
  stream_ << ";";
}

void Formatter::VisitField(ast::Field* field) {
  Indent();
  Visit(field->type());
  stream_ << " " << field->name();
  if (auto const expression = field->expression()) {
    stream_ << " = ";
    Visit(expression);
  }
  stream_ << ";" << std::endl;
}

void Formatter::VisitForEachStatement(ast::ForEachStatement* statement) {
  stream_ << "for (";
  Visit(statement->variable()->type());
  stream_ << " " << statement->variable()->name() << " : ";
  Visit(statement->enumerable());
  stream_ << ")";
  FormatChildStatement(statement->statement());
}

void Formatter::VisitForStatement(ast::ForStatement* statement) {
  stream_ << "for (";
  Visit(statement->initializer());
  if (statement->initializer()->is<ast::ExpressionList>())
    stream_ << ";";
  if (statement->condition()) {
    stream_ << " ";
    Visit(statement->condition());
  }
  stream_ << ";";
  if (statement->step()) {
    stream_ << " ";
    Visit(statement->step());
  }
  stream_ << ")";
  FormatChildStatement(statement->statement());
}

void Formatter::VisitIfStatement(ast::IfStatement* statement) {
  stream_ << "if (";
  Visit(statement->condition());
  stream_ << ")";
  if (!statement->else_statement()) {
    if (statement->then_statement()->is<ast::BlockStatement>()) {
      stream_ << " ";
      Visit(statement->then_statement());
      return;
    }
    stream_ << std::endl;
    IndentPlusOne();
    Visit(statement->then_statement());
    return;
  }

  auto const use_brace =
      statement->then_statement()->is<ast::BlockStatement>() ||
      statement->else_statement()->is<ast::BlockStatement>();

  if (statement->then_statement()->is<ast::BlockStatement>()) {
    stream_ << " ";
    Visit(statement->then_statement());
  } else if (use_brace) {
    stream_ << " {" << std::endl;
    IndentPlusOne();
    Visit(statement->then_statement());
    stream_ << std::endl;
    Indent();
    stream_ << "}";
  }

  if (statement->else_statement()->is<ast::BlockStatement>()) {
    stream_ << " else ";
    Visit(statement->else_statement());
    return;
  }

  if (use_brace) {
    stream_ << " else {" << std::endl;
    IndentPlusOne();
    Visit(statement->else_statement());
    stream_ << std::endl;
    Indent();
    stream_ << "}";
    return;
  }

  stream_ << std::endl;
  Indent();
  stream_ << "else" << std::endl;
  IndentPlusOne();
  Visit(statement->then_statement());
}

void Formatter::VisitImport(ast::Import* import) {
  Indent();
  stream_ << import->keyword() << " ";
  Visit(import->reference());
  stream_ << ";" << std::endl;
}

void Formatter::VisitInvalidExpression(ast::InvalidExpression* expression) {
  stream_ << "INVALID('" << expression->token() << "')";
}

void Formatter::VisitInvalidStatement(ast::InvalidStatement* statement) {
  stream_ << "INVALID '" << statement->token() << "';";
}

void Formatter::VisitLiteral(ast::Literal* operation) {
  stream_ << operation->token();
}

void Formatter::VisitMemberAccess(ast::MemberAccess* member_access) {
  const char* separator = "";
  for (auto const component : member_access->components()) {
    stream_ << separator;
    Visit(component);
    separator = ".";
  }
}

void Formatter::VisitMethod(ast::Method* method) {
  DCHECK(method);
}

void Formatter::VisitMethodGroup(ast::MethodGroup* method_group) {
  for (auto const method : method_group->methods()) {
    Indent();
    stream_ << method->modifiers();
    if (method->modifiers().value())
      stream_ << " ";
    Visit(method->return_type());
    stream_ << " " << method->name();
    if (!method->type_parameters().empty()) {
      const char* separator = "<";
      for (auto const name : method->type_parameters()) {
        stream_ << separator << name;
        separator = ", ";
      }
      stream_ << ">";
    }
    stream_ << "(";
    const char* separator = "";
    for (auto const param : method->parameters()) {
      stream_ << separator;
      Visit(param->type());
      stream_ << " " << param->name();
      separator = ", ";
    }
    stream_ << ")";
    auto const statement = method->body();
    if (!statement) {
      stream_ << ";" << std::endl;
      continue;
    }
    if (statement->is<ast::BlockStatement>()) {
      stream_ << " ";
      Visit(statement);
      stream_ << std::endl;
      continue;
    }
    stream_ << "=> ";
    Visit(statement);
    stream_ << ";" << std::endl;
  }
}

void Formatter::VisitNameReference(ast::NameReference* operation) {
  stream_ << operation->token();
}

void Formatter::VisitNamespace(ast::Namespace* ns) {
  for (auto const body : ns->bodies()) {
    Indent();
    stream_ << ns->token() << " " << ns->name() << " ";
    FormatBlock block(this);
    for (auto const member : body->members())
      Visit(member);
  }
}

void Formatter::VisitReturnStatement(ast::ReturnStatement* return_statement) {
  stream_ << "return";
  if (auto const value = return_statement->value()) {
    stream_ << " ";
    Visit(value);
  }
  stream_ << ";";
}

void Formatter::VisitThrowStatement(ast::ThrowStatement* throw_statement) {
  stream_ << "throw";
  if (auto const value = throw_statement->value()) {
    stream_ << " ";
    Visit(value);
  }
  stream_ << ";";
}

void Formatter::VisitTryStatement(ast::TryStatement* try_statement) {
  stream_ << "try ";
  Visit(try_statement->protected_block());
  for (auto const catch_clause : try_statement->catch_clauses()) {
    stream_ << " catch (";
    Visit(catch_clause->type());
    if (auto const variable = catch_clause->variable())
      stream_ << " " << variable->name();
    stream_ << ") ";
    Visit(catch_clause->block());
  }
  auto const finally_block = try_statement->finally_block();
  if (!finally_block)
    return;
  stream_ << " finally ";
  Visit(finally_block);
}

void Formatter::VisitUnaryOperation(ast::UnaryOperation* operation) {
  if (operation->op() == TokenType::PostDecrement ||
      operation->op() == TokenType::PostIncrement) {
    Visit(operation->expression());
    stream_ << operation->op();
    return;
  }
  stream_ << operation->op();
  Visit(operation->expression());
}

void Formatter::VisitUsingStatement(ast::UsingStatement* using_statement) {
  stream_ << "using (";
  if (auto const var = using_statement->variable()) {
    stream_ << "var ";
    stream_ << var->name();
    stream_ << " = ";
  }
  Visit(using_statement->resource());
  stream_ << ")";
  if (using_statement->statement()->is<ast::BlockStatement>()) {
    stream_ << " ";
    Visit(using_statement->statement());
  } else {
    stream_ << std::endl;
    Indent();
    Visit(using_statement->statement());
  }
}

void Formatter::VisitVarStatement(ast::VarStatement* var_statement) {
  if (var_statement->keyword() == TokenType::Const)
    stream_ << "const ";
  auto is_first = true;
  for (auto const var : var_statement->variables()) {
    if (is_first) {
      Visit(var->type());
      stream_ << " ";
      is_first = false;
    } else {
      stream_ << ", ";
    }
    stream_ << var->name();
    if (auto const value = var->value()) {
      stream_ << " = ";
      Visit(value);
    }
  }
  stream_ << ";";
}

void Formatter::VisitVariableReference(ast::VariableReference* var) {
  stream_ << var->token();
}

void Formatter::VisitWhileStatement(ast::WhileStatement* while_statement) {
  stream_ << "while (";
  Visit(while_statement->condition());
  stream_ << ") ";
  Visit(while_statement->statement());
}

void Formatter::VisitYieldStatement(ast::YieldStatement* yield_statement) {
  stream_ << "yield ";
  Visit(yield_statement->value());
  stream_ << ";";
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
