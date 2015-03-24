// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/optimizer/formatters/text_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/base/atomic_string.h"
#include "elang/optimizer/instructions.h"
#include "elang/optimizer/values.h"
#include "elang/optimizer/value_visitor.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_visitor.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(std::ostream* ostream) : ostream_(*ostream) {
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::FormatFunction(const Function* function) {
  ostream_ << *function << " " << *function->type() << std::endl;
}

}  // namespace optimizer
}  // namespace elang
