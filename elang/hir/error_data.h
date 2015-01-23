// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ERROR_DATA_H_
#define ELANG_HIR_ERROR_DATA_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

class Editor;
enum class ErrorCode;
class Value;

//////////////////////////////////////////////////////////////////////
//
// ErrorData
//
class ELANG_HIR_EXPORT ErrorData final : public ZoneAllocated {
 public:
  const ZoneVector<Value*>& details() const { return details_; }
  ErrorCode error_code() const { return error_code_; }
  Value* error_value() const { return error_value_; }

 private:
  friend class Editor;

  // |error_value| has error of |error_code| with additional information
  // in |details|.
  ErrorData(Zone* zone,
            ErrorCode error_code,
            Value* error_value,
            const std::vector<Value*>& details);
  ~ErrorData() = delete;

  const ZoneVector<Value*> details_;
  ErrorCode const error_code_;
  Value* const error_value_;

  DISALLOW_COPY_AND_ASSIGN(ErrorData);
};

std::ostream& operator<<(std::ostream& ostream, const ErrorData& error_data);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ERROR_DATA_H_
