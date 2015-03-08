// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ERROR_DATA_H_
#define ELANG_LIR_ERROR_DATA_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Editor;
enum class ErrorCode;
class LiteralMap;

//////////////////////////////////////////////////////////////////////
//
// ErrorData
//
class ELANG_LIR_EXPORT ErrorData final : public ZoneAllocated {
 public:
  const ZoneVector<Value>& details() const { return details_; }
  ErrorCode error_code() const { return error_code_; }
  Value error_value() const { return error_value_; }
  LiteralMap* literals() const { return literals_; }

 private:
  friend class Factory;

  // |error_value| has error of |error_code| with additional information
  // in |details|.
  ErrorData(Zone* zone,
            LiteralMap* literals,
            ErrorCode error_code,
            Value error_value,
            const std::vector<Value>& details);
  ~ErrorData() = delete;

  const ZoneVector<Value> details_;
  ErrorCode const error_code_;
  Value const error_value_;
  LiteralMap* const literals_;

  DISALLOW_COPY_AND_ASSIGN(ErrorData);
};

ELANG_LIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const ErrorData& error_data);

}  // namespace lir
}  // namespace elang

namespace std {

ELANG_LIR_EXPORT ostream& operator<<(
    ostream& ostream,
    const std::vector<elang::lir::ErrorData*>& errors);

}  // namespace std

#endif  // ELANG_LIR_ERROR_DATA_H_
