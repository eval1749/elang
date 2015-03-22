// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_ERROR_SINK_H_
#define ELANG_OPTIMIZER_ERROR_SINK_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_owner.h"

namespace elang {
namespace optimizer {

enum class ErrorCode;
class ErrorData;
class Node;
class Thing;

//////////////////////////////////////////////////////////////////////
//
// ErrorSink
//
class ErrorSink : public ZoneOwner {
 public:
  ~ErrorSink();

  const std::vector<ErrorData*>& errors() const { return errors_; }

  void AddError(ErrorCode error_code,
                const Node* node,
                const std::vector<Thing*>& details);

 protected:
  ErrorSink();

 private:
  std::vector<ErrorData*> errors_;

  DISALLOW_COPY_AND_ASSIGN(ErrorSink);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_ERROR_SINK_H_
