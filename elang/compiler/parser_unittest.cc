// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "base/macros.h"
#include "elang/compiler/ast/alias.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/namespace_body.h"
#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

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
  private: void Print(const ast::Alias* ns);
  private: void Print(const ast::Namespace* ns);
  private: void Print(const ast::NamespaceMember* member);
  private: void Print(const QualifiedName& ns);
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
      " = ";
  Print(alias->target_name());
  stream_ << ";" << std::endl;
}

void Formatter::Print(const ast::Namespace* ns) {
  for (auto const namespace_body : ns->bodies()) {
    Indent();
    stream_ << ns->token() << " " << ns->simple_name() <<
        " {" << std::endl;
    ++depth_;
    for (auto const member : namespace_body->members())
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
  if (auto const ns = member->as<ast::Namespace>()) {
    Print(ns);
    return;
  }
  Indent();
  stream_ << "NYI" << member->token() << std::endl;
}

void Formatter::Print(const QualifiedName& name) {
  const char* dot = "";
  for (const auto& simple_name : name.simple_names()) {
    stream_ << dot << simple_name;
    dot = ".";
  }
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
  if (!parser_.Run()) {
    // TODO(eval1749) Return error list.
    return std::string();
  }
  Formatter formatter;
  return formatter.Run(session_.global_namespace());
}

}  // namespace

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
