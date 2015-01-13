// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_COLLECTABLE_H_
#define ELANG_VM_COLLECTABLE_H_

#include "base/macros.h"

namespace elang {
namespace vm {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Collectable
//
class Collectable {
 public:
  void* operator new(size_t size, Factory* factory);

  // |Collectable| can't have |delete| operator. But MSVC requires them.
  void operator delete(void*, Factory*);

 protected:
  Collectable();

  // |Collectable| can't have destructor operator. But MSVC requires them.
  ~Collectable();

 private:
  DISALLOW_COPY_AND_ASSIGN(Collectable);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_COLLECTABLE_H_
