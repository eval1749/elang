// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_MACHINE_CODE_COLLECTION_H_
#define ELANG_VM_MACHINE_CODE_COLLECTION_H_

#include <map>
#include <unordered_map>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"

namespace elang {
class AtomicString;

namespace vm {
class Factory;
class MachineCodeFunction;

//////////////////////////////////////////////////////////////////////
//
// MachineCodeCollection
//
class MachineCodeCollection final {
 public:
  explicit MachineCodeCollection(Factory* factory);
  ~MachineCodeCollection();

  MachineCodeFunction* FunctionByAddress(uintptr_t address) const;
  MachineCodeFunction* FunctionByName(AtomicString* name) const;

 private:
  void InstallPredefinedFunction(Factory* factory,
                                 base::StringPiece name,
                                 uintptr_t entry_point);
  void RegisterFunction(AtomicString* name, MachineCodeFunction* function);

  Factory* const factory_;
  std::map<uintptr_t, MachineCodeFunction*> address_map_;
  std::unordered_map<AtomicString*, MachineCodeFunction*> name_map_;

  DISALLOW_COPY_AND_ASSIGN(MachineCodeCollection);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_MACHINE_CODE_COLLECTION_H_
