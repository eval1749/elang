// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_VALUE_EMITTER_H_
#define ELANG_LIR_EMITTERS_VALUE_EMITTER_H_

#include "base/macros.h"
#include "elang/lir/literal_visitor.h"

namespace elang {

namespace api {
class MachineCodeBuilder;
}

namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ValueEmitter
//
class ValueEmitter : private LiteralVisitor {
 public:
  ValueEmitter(Factory* factory, api::MachineCodeBuilder* builder);
  ~ValueEmitter() = default;

  void Emit(int offset, Value value);

 private:
// LiteralVisitor
#define V(Name, ...) void Visit##Name(Name* type) final;
  FOR_EACH_LIR_LITERAL(V)
#undef V

  api::MachineCodeBuilder* const builder_;
  Factory* factory_;
  int code_offset_;

  DISALLOW_COPY_AND_ASSIGN(ValueEmitter);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_VALUE_EMITTER_H_
