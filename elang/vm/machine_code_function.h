// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_MACHINE_CODE_FUNCTION_H_
#define ELANG_VM_MACHINE_CODE_FUNCTION_H_

#include "elang/api/machine_code_builder.h"

#include "elang/vm/collectable.h"
#include "elang/vm/entry_point.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// MachineCodeFunction
//
class MachineCodeFunction final : public Collectable {
 public:
  ~MachineCodeFunction() = delete;

  template <typename Return, typename... Params>
  Return Call(Params... params) {
    typedef Return(Signature)(Params...);
    return reinterpret_cast<Signature*>(entry_point_)(params...);
  }

  template <typename... Params>
  void Invoke(Params... params) {
    typedef void(Signature)(Params...);
    return reinterpret_cast<Signature*>(entry_point_)(params...);
  }

 private:
  friend class MachineCodeBuilderImpl;

  MachineCodeFunction(EntryPoint entry_point, int code_size);

  EntryPoint const entry_point_;
  int const code_size_;

  DISALLOW_COPY_AND_ASSIGN(MachineCodeFunction);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_MACHINE_CODE_FUNCTION_H_
