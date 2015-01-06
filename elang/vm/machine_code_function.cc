// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/machine_code_function.h"

#include "base/logging.h"

namespace elang {
namespace vm {

MachineCodeFunction::MachineCodeFunction(EntryPoint entry_point, int code_size)
    : entry_point_(entry_point), code_size_(code_size) {
  DCHECK(entry_point_);
}

}  // namespace vm
}  // namespace elang
