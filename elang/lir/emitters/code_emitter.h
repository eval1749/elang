// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_CODE_EMITTER_H_
#define ELANG_LIR_EMITTERS_CODE_EMITTER_H_

#include <memory>

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {

namespace api {
class MachineCodeBuilder;
}

namespace lir {

class CodeBuffer;
class Factory;
class Function;
class InstructionVisitor;

//////////////////////////////////////////////////////////////////////
//
// CodeEmitter
//
class ELANG_LIR_EXPORT CodeEmitter final {
 public:
  CodeEmitter(const Factory* factory, api::MachineCodeBuilder* builder);
  ~CodeEmitter();

  void Process(const Function* function);

 private:
  // Returns target specific machine code emitter for instructions.
  // Implementation of |NewInstructionHandler| found in
  // "code_emitter_${arch}.cc".
  std::unique_ptr<InstructionVisitor> NewInstructionHandler(
      CodeBuffer* code_buffer);

  // Machine codes and embedded literals are feed to |builder_|.
  api::MachineCodeBuilder* const builder_;

  // |factory_| provides information for literals.
  const Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(CodeEmitter);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_CODE_EMITTER_H_
