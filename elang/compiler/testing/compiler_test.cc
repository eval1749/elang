// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/compiler_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/query/node_queries.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/compiler/testing/formatter.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::string ConvertErrorListToString(const std::vector<ErrorData*> errors) {
  static const char* const mnemonic[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_COMPILER_ERROR_CODE(V, V)
#undef V
  };

  std::ostringstream stream;
  for (auto const error : errors) {
    auto const index = static_cast<int>(error->error_code());
    stream << mnemonic[index] << "(" << error->location().start().offset()
           << ")";
    for (auto token : error->tokens())
      stream << " " << token;
    stream << std::endl;
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

// Since we don't want to include "string_source_code.h" in "compiler_test.h",
// we implement |source_code()| function here.
SourceCode* CompilerTest::source_code() const {
  return source_codes_.back().get();
}

ast::Node* CompilerTest::FindMember(base::StringPiece qualified_name) {
  return session()->QueryAstNode(base::UTF8ToUTF16(qualified_name));
}

std::string CompilerTest::Format(base::StringPiece source_code) {
  Prepare(source_code);
  return Format();
}

std::string CompilerTest::Format() {
  if (!Parse())
    return GetErrors();
  Formatter formatter;
  return formatter.Run(session_.get());
}

std::string CompilerTest::GetErrors() {
  return ConvertErrorListToString(session_->errors());
}

std::string CompilerTest::GetWarnings() {
  return ConvertErrorListToString(session_->warnings());
}

bool CompilerTest::Parse() {
  for (auto const& source_code : source_codes_) {
    auto const compilation_unit =
        session_->NewCompilationUnit(source_code.get());
    compilation_units_.push_back(compilation_unit);
    Parser(session_.get(), compilation_unit).Run();
  }
  return !session_->HasError();
}

void CompilerTest::Prepare(base::StringPiece16 source_text) {
  source_codes_.push_back(
      std::make_unique<StringSourceCode>(L"testing", source_text));
}

void CompilerTest::Prepare(base::StringPiece source_text) {
  Prepare(base::UTF8ToUTF16(source_text));
}

std::vector<ast::Node*> CompilerTest::QueryAstNodes(TokenType token_type) {
  return session()->QueryAstNodes(ast::TokenTypeQuery(token_type));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
