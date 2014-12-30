// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/testing/compiler_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/testing/formatter.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::string GetQualifiedName(ast::NamespaceMember* member) {
  std::vector<Token*> names;
  names.push_back(member->name());
  for (auto ns = member->outer(); ns && ns->outer(); ns = ns->outer())
    names.push_back(ns->name());
  std::reverse(names.begin(), names.end());
  std::stringstream stream;
  const char* separator = "";
  for (auto const name : names) {
    stream << separator << name;
    separator = ".";
  }
  return stream.str();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompilerTest
//
CompilerTest::CompilerTest() : session_(new CompilationSession()) {
}

CompilerTest::~CompilerTest() {
}

ast::Class* CompilerTest::FindClass(base::StringPiece name) {
  auto const member = FindMember(name);
  return member ? member->as<ast::Class>() : nullptr;
}

ast::NamespaceMember* CompilerTest::FindMember(base::StringPiece name) {
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

std::string CompilerTest::GetBaseClasses(base::StringPiece name) {
  auto const member = FindMember(name);
  if (!member)
    return base::StringPrintf("No such class %s", name);
  auto const clazz = member->as<ast::Class>();
  if (!clazz)
    return base::StringPrintf("%s isn't class", name);
#if 0
  std::stringstream stream;
  const char* separator = "";
  for (auto base_class : clazz->base_classes()) {
    stream << separator << GetQualifiedName(base_class);
    separator = ", ";
  }
  return stream.str();
#endif
  return "NYI";
}

std::string CompilerTest::GetErrors() {
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

std::string CompilerTest::AnalyzeNamespace() {
  if (!Parse())
    return GetErrors();
  NamespaceAnalyzer resolver(session_.get());
  return resolver.Run() ? "" : GetErrors();
}

std::string CompilerTest::Format() {
  if (!Parse())
    return GetErrors();
  Formatter formatter;
  return formatter.Run(session_->global_namespace());
}

bool CompilerTest::Parse() {
  auto const compilation_unit =
      session_->NewCompilationUnit(source_code_.get());
  Parser parser(session_.get(), compilation_unit);
  return parser.Run();
}

void CompilerTest::Prepare(base::StringPiece source_text) {
  source_code_.reset(
      new StringSourceCode(L"testing", base::UTF8ToUTF16(source_text)));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
