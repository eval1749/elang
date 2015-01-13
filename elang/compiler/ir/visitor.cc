// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ir/visitor.h"

#include "base/logging.h"

namespace elang {
namespace compiler {
namespace ir {

//////////////////////////////////////////////////////////////////////
//
// Visitor
//
Visitor::Visitor() {
}

Visitor::~Visitor() {
}

// Default implementations do nothing. Each derived visitor class implements
// override for interested classes.
//
// To avoid including header files of concrete class here, |Accept(Visitor)|
// functions are implemented in "factory.cc".
#define V(Name) \
  void Visitor::Visit##Name(Name* node) { DCHECK(node); }
  FOR_EACH_CONCRETE_IR_NODE(V)
#undef V

}  // namespace ir
}  // namespace compiler
}  // namespace elang
