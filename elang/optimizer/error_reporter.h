// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_ERROR_REPORTER_H_
#define ELANG_OPTIMIZER_ERROR_REPORTER_H_

#include "base/basictypes.h"

namespace elang {
namespace optimizer {

enum class ErrorCode;
class ErrorData;
class Factory;
class Node;
class Thing;

//////////////////////////////////////////////////////////////////////
//
// ErrorReporter
//
class ErrorReporter {
 protected:
  explicit ErrorReporter(Factory* factory);
  ~ErrorReporter();

  // Validation errors
  void Error(ErrorCode error_code, Node* error_node);
  void Error(ErrorCode error_code, Node* error_node, Thing* detail);
  void Error(ErrorCode error_code,
             Node* error_node,
             Thing* detail1,
             Thing* detail2);

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(ErrorReporter);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_ERROR_REPORTER_H_
