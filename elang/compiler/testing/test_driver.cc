// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/testing/test_driver.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/enum_member.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/unary_operation.h"
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

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public ast::Visitor {
  private: class FormatBlock {
    private: Formatter* formatter_;

    public: FormatBlock(Formatter* formatter);
    public: ~FormatBlock();

    DISALLOW_COPY_AND_ASSIGN(FormatBlock);
  };
  friend class FormatBlock;

  private: std::stringstream stream_;
  private: int depth_;

  public: Formatter();
  public: ~Formatter() = default;

  private: void Indent();
  public: std::string Run(ast::Namespace* ns);

  // ast::Visitor
  private: void Visit(ast::Node* node) final;

  #define DECLARE_VISIT(type) \
    private: void Visit##type(ast::type* node) final;
  AST_NODE_LIST(DECLARE_VISIT)
  #undef DECLARE_VISIT

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

//////////////////////////////////////////////////////////////////////
//
// Formatter::FormatBlock
//
Formatter::FormatBlock::FormatBlock(Formatter* formatter)
    : formatter_(formatter) {
  formatter_->stream_ << " {" << std::endl;
  ++formatter_->depth_;
}

Formatter::FormatBlock::~FormatBlock() {
  --formatter_->depth_;
  formatter_->Indent();
  formatter_->stream_ << "}" << std::endl;
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
  stream_ << alias->token() << " " << alias->simple_name() <<
      " = " << alias->target_name() << ";" << std::endl;
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

    FormatBlock block(this);
    for (auto const member : body->members())
      Visit(member);
  }
}

void Formatter::VisitEnum(ast::Enum* enumx) {
  Indent();
  stream_ << "enum " << enumx->simple_name();
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

void Formatter::VisitLiteral(ast::Literal* operation) {
  stream_ << operation->token();
}

void Formatter::VisitNameReference(ast::NameReference* operation) {
  stream_ << operation->token();
}

void Formatter::VisitNamespace(ast::Namespace* ns) {
  for (auto const body : ns->bodies()) {
    Indent();
    stream_ << ns->token() << " " << ns->simple_name();
    FormatBlock block(this);
    for (auto const member : body->members())
      Visit(member);
  }
}

void Formatter::VisitUnaryOperation(ast::UnaryOperation* operation) {
  if (operation->op()->type() == TokenType::PostDecrement ||
      operation->op()->type() == TokenType::PostIncrement) {
    Visit(operation->expression());
    stream_ << operation->op();
    return;
  }
  stream_ << operation->op();
  Visit(operation->expression());
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TestDriver
//
TestDriver::TestDriver(base::StringPiece source_text)
    : session_(new CompilationSession()),
      source_code_(new StringSourceCode(L"testing",
                                        base::UTF8ToUTF16(source_text))),
      compilation_unit_(new CompilationUnit(session_.get(),
                                            source_code_.get())) {
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
    #define E(category, subcategory, name) \
        #category "." #subcategory "." #name,
    COMPILER_ERROR_CODE_LIST(E, E)
    #undef E
  };

  std::stringstream stream;
  for (auto const error : session_->errors()) {
    stream << error_messages[static_cast<int>(error->error_code())] << "(" <<
        error->location().start().offset() << ")";
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
