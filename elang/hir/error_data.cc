// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/hir/error_data.h"

#include "elang/hir/error_code.h"
#include "elang/hir/thing.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {

ErrorData::ErrorData(Zone* zone,
                     ErrorCode error_code,
                     Value* error_value,
                     const std::vector<Thing*>& details)
    : details_(zone, details),
      error_code_(error_code),
      error_value_(error_value) {
}

std::ostream& operator<<(std::ostream& ostream, const ErrorData& error) {
  static const char* const mnemonics[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_HIR_ERROR_CODE(V, V)
#undef V
          "Invalid",
  };

  auto const index = std::min(static_cast<size_t>(error.error_code()),
                              arraysize(mnemonics) - 1);
  ostream << mnemonics[index] << "(" << *error.error_value();
  for (auto detail : error.details())
    ostream << " " << *detail;
  return ostream << ")";
}

}  // namespace hir
}  // namespace elang

namespace std {
ostream& operator<<(ostream& ostream,
                    const vector<elang::hir::ErrorData*>& errors) {
  for (auto const error : errors)
    ostream << *error << std::endl;
  return ostream;
}
}  // namespace std
