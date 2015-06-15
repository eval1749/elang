// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_DISASM_H_
#define ELANG_SHELL_DISASM_H_

#include <iosfwd>

namespace elang {
namespace vm {
class MachineCodeFunction;
}
namespace compiler {
namespace shell {

struct DisassembleMachineCodeFunction {
  const vm::MachineCodeFunction* function;
};

std::ostream& operator<<(std::ostream& ostream,
                         const DisassembleMachineCodeFunction& function);

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_DISASM_H_
