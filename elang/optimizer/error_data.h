// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_ERROR_DATA_H_
#define ELANG_OPTIMIZER_ERROR_DATA_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class ErrorSink;
enum class ErrorCode;
class Node;
class Thing;

//////////////////////////////////////////////////////////////////////
//
// ErrorData
//
class ELANG_OPTIMIZER_EXPORT ErrorData final : public ZoneAllocated {
 public:
  const ZoneVector<Thing*>& details() const { return details_; }
  ErrorCode error_code() const { return error_code_; }
  Node* error_value() const { return error_value_; }

 private:
  friend class ErrorSink;

  // |error_value| has error of |error_code| with additional information
  // in |details|.
  ErrorData(Zone* zone,
            ErrorCode error_code,
            Node* error_value,
            const std::vector<Thing*>& details);
  ~ErrorData() = delete;

  const ZoneVector<Thing*> details_;
  ErrorCode const error_code_;
  Node* const error_value_;

  DISALLOW_COPY_AND_ASSIGN(ErrorData);
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const ErrorData& error_data);

}  // namespace optimizer
}  // namespace elang

namespace std {

ELANG_OPTIMIZER_EXPORT ostream& operator<<(
    ostream& ostream,
    const std::vector<elang::optimizer::ErrorData*>& errors);

}  // namespace std

#endif  // ELANG_OPTIMIZER_ERROR_DATA_H_
