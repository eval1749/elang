// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_VALIDATOR_H_
#define ELANG_OPTIMIZER_VALIDATOR_H_

#include <vector>

#include "base/macros.h"
#include "elang/optimizer/error_reporter.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class Factory;
class Function;

//////////////////////////////////////////////////////////////////////
//
// Validator
//
class ELANG_OPTIMIZER_EXPORT Validator final : public ErrorReporter {
 public:
  Validator(Factory* factory, const Function* function);
  ~Validator();

  bool Validate(const Node* node);
  bool Validate();

 private:
  class Context;

  Node* NewInt32(int data);

  Factory* const factory_;
  const Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(Validator);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_VALIDATOR_H_
