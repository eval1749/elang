// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/optimizer/error_data.h"

#include "elang/optimizer/error_code.h"
#include "elang/optimizer/thing.h"
#include "elang/optimizer/nodes_forward.h"

namespace elang {
namespace optimizer {

ErrorData::ErrorData(Zone* zone,
                     ErrorCode error_code,
                     Node* error_value,
                     const std::vector<Thing*>& details)
    : details_(zone, details),
      error_code_(error_code),
      error_value_(error_value) {
}

std::ostream& operator<<(std::ostream& ostream, const ErrorData& error) {
  static const char* const mnemonics[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_OPTIMIZER_ERROR_CODE(V, V)
#undef V
  };

  auto const it =
      std::begin(mnemonics) + static_cast<size_t>(error.error_code());
  auto const mnemonic = it < std::end(mnemonics) ? *it : "Invalid";
  ostream << mnemonic << "(" << *error.error_value();
  for (auto detail : error.details())
    ostream << " " << *detail;
  return ostream << ")";
}

}  // namespace optimizer
}  // namespace elang

namespace std {
ostream& operator<<(ostream& ostream,
                    const vector<elang::optimizer::ErrorData*>& errors) {
  for (auto const error : errors)
    ostream << *error << std::endl;
  return ostream;
}
}  // namespace std
