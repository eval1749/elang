// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/compiler_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/namespace.h"
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

  std::stringstream stream;
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
  return source_code_.get();
}

std::string CompilerTest::Format(base::StringPiece source_code) {
  Prepare(source_code);
  return Format();
}

std::string CompilerTest::Format() {
  if (!Parse())
    return GetErrors();
  Formatter formatter;
  return formatter.Run(session_->global_namespace_body());
}

std::string CompilerTest::GetErrors() {
  return ConvertErrorListToString(session_->errors());
}

std::string CompilerTest::GetWarnings() {
  return ConvertErrorListToString(session_->warnings());
}

bool CompilerTest::Parse() {
  auto const compilation_unit = session_->NewCompilationUnit(source_code());
  Parser parser(session_.get(), compilation_unit);
  return parser.Run();
}

void CompilerTest::Prepare(base::StringPiece16 source_text) {
  source_code_.reset(new StringSourceCode(L"testing", source_text));
}

void CompilerTest::Prepare(base::StringPiece source_text) {
  Prepare(base::UTF8ToUTF16(source_text));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
