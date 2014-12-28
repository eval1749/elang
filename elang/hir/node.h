// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_node_h)
#define INCLUDE_elang_hir_node_h

#include "elang/base/castable.h"

#include "base/macros.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable {
  DECLARE_CASTABLE_CLASS(Node, Castable);
  friend class Factory;

 protected:
  Node();
  virtual ~Node();

 private:
  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace hir
}  // namespace elang

#endif  // !defined(INCLUDE_elang_hir_node_h)
