// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/lir/error_data.h"

#include "elang/lir/error_code.h"
#include "elang/lir/literals.h"
#include "elang/lir/printable.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

ErrorData::ErrorData(Zone* zone,
                     LiteralMap* literals,
                     ErrorCode error_code,
                     Value error_value,
                     const std::vector<Value>& details)
    : details_(zone, details),
      error_code_(error_code),
      error_value_(error_value),
      literals_(literals) {
}

std::ostream& operator<<(std::ostream& ostream, const ErrorData& error) {
  static const char* const mnemonics[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_LIR_ERROR_CODE(V, V)
#undef V
          "Invalid",
  };

  auto const index = std::min(static_cast<size_t>(error.error_code()),
                              arraysize(mnemonics) - 1);
  auto const literals = error.literals();
  ostream << mnemonics[index] << "("
          << AsPrintableValue(literals, error.error_value());
  for (auto detail : error.details())
    ostream << " " << AsPrintableValue(literals, detail);
  return ostream << ")";
}

}  // namespace lir
}  // namespace elang

namespace std {
ostream& operator<<(ostream& ostream,
                    const vector<elang::lir::ErrorData*>& errors) {
  for (auto const error : errors)
    ostream << *error << std::endl;
  return ostream;
}
}  // namespace std
