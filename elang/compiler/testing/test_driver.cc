// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/testing/test_driver.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/array_type.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/block_statement.h"
#include "elang/compiler/ast/break_statement.h"
#include "elang/compiler/ast/catch_clause.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/continue_statement.h"
#include "elang/compiler/ast/do_statement.h"
#include "elang/compiler/ast/empty_statement.h"
#include "elang/compiler/ast/expression_statement.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/field.h"
#include "elang/compiler/ast/if_statement.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/member_access.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/method_group.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/return_statement.h"
#include "elang/compiler/ast/throw_statement.h"
#include "elang/compiler/ast/try_statement.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/ast/var_statement.h"
#include "elang/compiler/ast/while_statement.h"
#include "elang/compiler/ast/yield_statement.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/name_resolver.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/parser.h"
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

namespace {

enum class NewlineAtEnd {
  No,
  Yes,
};

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public ast::Visitor {
 public:
  Formatter();
  ~Formatter() = default;

  std::string Run(ast::Namespace* ns);

 private:
  class FormatBlock {
   public:
    explicit FormatBlock(Formatter* formatter,
                         NewlineAtEnd newline_at_end = NewlineAtEnd::Yes);
    ~FormatBlock();

   private:
    Formatter* const formatter_;
    NewlineAtEnd const newline_at_end_;

    DISALLOW_COPY_AND_ASSIGN(FormatBlock);
  };
  friend class FormatBlock;

  void Indent();
  void IndentPlusOne();

  // ast::Visitor
  void Visit(ast::Node* node) final;

#define DECLARE_VISIT(type) void Visit##type(ast::type* node) final;
  AST_NODE_LIST(DECLARE_VISIT)
#undef DECLARE_VISIT

  std::stringstream stream_;
  int depth_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

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
  stream_ << alias->token() << " " << alias->simple_name() << " = "
          << alias->target_name() << ";" << std::endl;
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
    stream_ << klass->token() << " " << klass->simple_name();
    const char* separator = " : ";
    for (auto const base_class_name : klass->base_class_names()) {
      stream_ << separator << base_class_name;
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
  stream_ << "enum " << enumx->simple_name() << " ";
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

void Formatter::VisitLiteral(ast::Literal* operation) {
  stream_ << operation->token();
}

void Formatter::VisitMemberAccess(ast::MemberAccess* member_access) {
  const char* separator = "";
  for (auto const member : member_access->members()) {
    stream_ << separator;
    Visit(member);
    separator = ".";
  }
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
    auto const statement = method->statement();
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
    stream_ << ns->token() << " " << ns->simple_name() << " ";
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

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TestDriver
//
TestDriver::TestDriver(base::StringPiece source_text)
    : session_(new CompilationSession()),
      source_code_(
          new StringSourceCode(L"testing", base::UTF8ToUTF16(source_text))),
      compilation_unit_(
          new CompilationUnit(session_.get(), source_code_.get())) {
}

TestDriver::~TestDriver() {
}

ast::Class* TestDriver::FindClass(base::StringPiece name) {
  auto const member = FindMember(name);
  return member ? member->as<ast::Class>() : nullptr;
}

ast::NamespaceMember* TestDriver::FindMember(base::StringPiece name) {
  auto enclosing = session_->global_namespace();
  auto found = static_cast<ast::NamespaceMember*>(nullptr);
  for (size_t pos = 0u; pos < name.length(); ++pos) {
    auto dot_pos = name.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = name.length();
    auto const simple_name = session_->GetOrCreateSimpleName(
        base::UTF8ToUTF16(name.substr(pos, dot_pos - pos)));
    found = enclosing->FindMember(simple_name);
    if (!found)
      return nullptr;
    enclosing = found->as<ast::Namespace>();
    if (!enclosing)
      return nullptr;
    pos = dot_pos;
  }
  return found;
}

std::string TestDriver::GetErrors() {
  static const char* const error_messages[] = {
#define E(category, subcategory, name) #category "." #subcategory "." #name,
      COMPILER_ERROR_CODE_LIST(E, E)
#undef E
  };

  std::stringstream stream;
  for (auto const error : session_->errors()) {
    stream << error_messages[static_cast<int>(error->error_code())] << "("
           << error->location().start().offset() << ")";
    for (auto token : error->tokens())
      stream << " " << token;
    stream << std::endl;
  }
  return stream.str();
}

std::string TestDriver::RunNameResolver() {
  {
    Parser parser(session_.get(), compilation_unit_.get());
    if (!parser.Run())
      return GetErrors();
  }
  NameResolver resolver(session_.get());
  return resolver.Run() ? "" : GetErrors();
}

std::string TestDriver::RunParser() {
  Parser parser(session_.get(), compilation_unit_.get());
  if (parser.Run()) {
    Formatter formatter;
    return formatter.Run(session_->global_namespace());
  }
  return GetErrors();
}

}  // namespace compiler
}  // namespace elang
