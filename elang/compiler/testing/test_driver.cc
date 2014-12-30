// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/testing/test_driver.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
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

}  // namespace testing
}  // namespace compiler
}  // namespace elang
