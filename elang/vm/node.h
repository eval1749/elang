// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_NODE_H_
#define ELANG_VM_NODE_H_

#include "base/macros.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"

namespace elang {
namespace vm {

#define DECLARE_VM_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);     \
  friend class Factory;                    \
                                           \
 protected:                                \
  ~self() override = default;              \
                                           \
 private:
//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable<Node>, public ZoneAllocated {
  DECLARE_VM_NODE_CLASS(Node, Castable);

 protected:
  Node();

 private:
  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_NODE_H_
