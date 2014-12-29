// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/test_driver.h"

#include "base/logging.h"
#include "base/macros.h"
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

std::string TestDriver::RunNamespaceAnalyzer() {
  {
    Parser parser(session_.get(), compilation_unit_.get());
    if (!parser.Run())
      return GetErrors();
  }
  NamespaceAnalyzer resolver(session_.get());
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

}  // namespace testing
}  // namespace compiler
}  // namespace elang
