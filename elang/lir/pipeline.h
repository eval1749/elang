// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_PIPELINE_H_
#define ELANG_LIR_PIPELINE_H_

#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/lir/editor_user.h"
#include "elang/lir/factory_user.h"

namespace elang {

namespace api {
class MachineCodeBuilder;
class PassObserver;
}

namespace lir {

class Factory;
class Function;

//////////////////////////////////////////////////////////////////////
//
// Pipeline
//
class Pipeline final {
 public:
  Pipeline(Factory* factory,
           api::PassObserver* observer,
           api::MachineCodeBuilder* builder,
           Function* function);
  ~Pipeline();

  bool Run();

 private:
  api::MachineCodeBuilder* const builder_;
  Factory* const factory_;
  Function* const function_;
  api::PassObserver* observer_;

  DISALLOW_COPY_AND_ASSIGN(Pipeline);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_PIPELINE_H_
