// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "base/macros.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "testing/gtest/include/gtest/gtest.h"

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
class Formatter final {
  private: std::stringstream stream_;
  private: int depth_;

  public: Formatter();
  public: ~Formatter() = default;

  private: void Indent();
  private: void Print(const ast::Alias* alias);
  private: void Print(const ast::Class* klass);
  private: void Print(const ast::Namespace* ns);
  private: void Print(const ast::NamespaceMember* member);
  public: std::string Run(const ast::Namespace* ns);

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::Formatter() : depth_(0) {
}

void Formatter::Indent() {
  stream_ << std::string(depth_ * 2, ' ');
}

void Formatter::Print(const ast::Alias* alias) {
  Indent();
  stream_ << alias->token() << " " << alias->simple_name() <<
      " = " << alias->target_name() << ";" << std::endl;
}

void Formatter::Print(const ast::Class* klass) {
  for (auto const body : klass->bodies()) {
    Indent();
    stream_ << klass->token() << " " << klass->simple_name();
    const char* separator = " : ";
    for (auto const base_class_name : klass->base_class_names()) {
      stream_ << separator << base_class_name;
      separator = ", ";
    }
    stream_ << " {" << std::endl;
    ++depth_;
    for (auto const member : body->members())
      Print(member);
    --depth_;
    Indent();
    stream_ << "}" << std::endl;
  }
}

void Formatter::Print(const ast::Namespace* ns) {
  for (auto const body : ns->bodies()) {
    Indent();
    stream_ << ns->token() << " " << ns->simple_name() <<
        " {" << std::endl;
    ++depth_;
    for (auto const member : body->members())
      Print(member);
    --depth_;
    Indent();
    stream_ << "}" << std::endl;
  }
}

void Formatter::Print(const ast::NamespaceMember* member) {
  if (auto const alias = member->as<ast::Alias>()) {
    Print(alias);
    return;
  }
  if (auto const klass = member->as<ast::Class>()) {
    Print(klass);
    return;
  }
  if (auto const ns = member->as<ast::Namespace>()) {
    Print(ns);
    return;
  }
  Indent();
  stream_ << "NYI" << member->token() << std::endl;
}

std::string Formatter::Run(const ast::Namespace* ns) {
  stream_.clear();
  depth_ = 0;
  for (auto const namespace_body : ns->bodies()) {
    for (auto const member : namespace_body->members())
      Print(member);
  }
  return stream_.str();
}

//////////////////////////////////////////////////////////////////////
//
// TestParsr
//
class TestParser final {
  private: CompilationSession session_;
  private: StringSourceCode source_code_;
  private: CompilationUnit compilation_unit_;
  private: Parser parser_;

  public: TestParser(const base::string16& source);
  public: std::string Run();

  DISALLOW_COPY_AND_ASSIGN(TestParser);
};

TestParser::TestParser(const base::string16& string)
    : source_code_(L"sample", string),
      compilation_unit_(&session_, &source_code_),
      parser_(&session_, &compilation_unit_) {
}

std::string TestParser::Run() {
  if (parser_.Run()) {
    Formatter formatter;
    return formatter.Run(session_.global_namespace());
  }

  static const char* const error_messages[] = {
    #define E(category, subcategory, name) \
        #category "." #subcategory "." #name,
    COMPILER_ERROR_CODE_LIST(E, E)
    #undef E
  };

  std::stringstream stream;
  for (auto const error : session_.errors()) {
    stream << error_messages[static_cast<int>(error->error_code())] << "(" <<
        error->location().start().offset() << ")" << std::endl;
  }
  return stream.str();
}

}  // namespace

TEST(ParserTest, ClassBasic) {
  TestParser parser(
    L"class A : C {} class B : A {} class C {}");
  EXPECT_EQ(
    "class A : C {\n}\n"
    "class B : A {\n}\n"
    "class C {\n}\n", parser.Run());
}

TEST(ParserTest, NamespaceAlias) {
  TestParser parser(
    L"namespace A { using B = N1.N2; }");
  EXPECT_EQ(
    "namespace A {\n"
    "  using B = N1.N2;\n"
    "}\n", parser.Run());
}

TEST(ParserTest, NamespaceBasic) {
  TestParser parser(
    L"namespace A { namespace B { namespace C {} } }\n"
    L"namespace D {}");
  EXPECT_EQ(
    "namespace A {\n"
    "  namespace B {\n"
    "    namespace C {\n"
    "    }\n"
    "  }\n"
    "}\n"
    "namespace D {\n"
    "}\n", parser.Run());
}

}  // namespace compiler
}  // namespace elang
