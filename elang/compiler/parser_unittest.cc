// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/compiler/parser.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/namespace.h"
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

void Formatter::Print(const ast::Namespace* ns) {
  Indent();
  stream_ << "namespace ";
  Print(ns->name());
  stream_ << " {" << std::endl;
  ++depth_;
  for (auto const member : ns->members())
    Print(member);
  --depth_;
  Indent();
  stream_ << "}" << std::endl;
}

void Formatter::Print(const ast::NamespaceMember* member) {
  switch (member->token().type()) {
    case TokenType::Namespace:
      Print(reinterpret_cast<const ast::Namespace*>(member));
      return;
  }
  Indent();
  stream_ << "NYI" << member->token().type() << std::endl;
}

void Formatter::Print(const QualifiedName& name) {
  const char* dot = "";
  for (const auto& simple_name : name.simple_names()) {
    base::string16 string;
    simple_name.string_data().CopyToString(&string);
    stream_ << dot << base::UTF16ToUTF8(string);
    dot = ".";
  }
}

std::string Formatter::Run(const ast::Namespace* ns) {
  stream_.clear();
  depth_ = 0;
  for (auto member : ns->members()) {
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
  return formatter.Run(compilation_unit_.global_namespace());
}

char ToHexDigit(int n) {
  auto const n4 = n & 15;
  if (n4 < 10)
    return '0' + n4;
  return n4 + 'A' - 10;
}

void PrintTo(const Token& token, ::std::ostream* ostream) {
  static const char* kTokenTypeString[] = {
    #define V(name, string, details) string,
    TOKEN_LIST(V, V)
  };
  *ostream << "Token(" << kTokenTypeString[static_cast<int>(token.type())];
  switch (token.type()) {
    case TokenType::Float32Literal:
      *ostream << " " << token.f32_data();
      break;
    case TokenType::Float64Literal:
      *ostream << " " << token.f64_data();
      break;
    case TokenType::Int32Literal:
    case TokenType::Int64Literal:
    case TokenType::UInt32Literal:
    case TokenType::UInt64Literal:
      *ostream << " " << token.int64_data();
      break;
    case TokenType::StringLiteral:
      *ostream << " \"";
      for (auto const ch : token.string_data()) {
        char buffer[7];
        if (ch < ' ' || ch >= 0x7F) {
          buffer[0] = '\\';
          buffer[1] = 'u';
          buffer[2] = ToHexDigit(ch >> 12);
          buffer[3] = ToHexDigit(ch >> 8);
          buffer[4] = ToHexDigit(ch >> 4);
          buffer[5] = ToHexDigit(ch);
          buffer[6] = 0;
        } else {
          buffer[0] = ch;
          buffer[1] = 0;
        }
        *ostream << buffer;
      }
      *ostream << "\"";
      break;

    default:
      if (token.is_name()) {
        base::string16 string;
        token.string_data().CopyToString(&string);
        *ostream << " " << base::UTF16ToUTF8(string);
      }
      break;
  }
  *ostream << " " << token.location().start().offset() <<
      " " << token.location().end().offset() << ")";
}

}  // namespace

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
