// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_CODE_EMITTER_H_
#define ELANG_LIR_CODE_EMITTER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {

namespace api {
class MachineCodeBuilder;
}

namespace lir {

class Factory;
class Function;

//////////////////////////////////////////////////////////////////////
//
// CodeEmitter
//
class ELANG_LIR_EXPORT CodeEmitter final {
 public:
  CodeEmitter(Factory* factory, api::MachineCodeBuilder* builder);
  ~CodeEmitter();

  void Process(const Function* function);

 private:
  api::MachineCodeBuilder* const builder_;
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(CodeEmitter);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_CODE_EMITTER_H_
