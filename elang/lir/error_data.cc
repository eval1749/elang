// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

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
  };

  auto const error_code = error.error_code();
  auto const it = std::begin(mnemonics) + static_cast<size_t>(error_code);
  auto const mnemonic = it < std::end(mnemonics) ? *it : "invalid";
  auto const literals = error.literals();
  ostream << mnemonic << "("
          << PrintableValue(literals, error.error_value());
  auto separator = ": ";
  for (auto detail : error.details()) {
    ostream << separator << PrintableValue(literals, detail);
    separator = ", ";
  }
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
