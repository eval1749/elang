// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_VISITOR_H_
#define ELANG_COMPILER_IR_VISITOR_H_

#include "base/macros.h"
#include "elang/compiler/ir/nodes_forward.h"

namespace elang {
namespace compiler {
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// Visitor
//
class Visitor {
 public:
  Visitor();
  virtual ~Visitor();

#define V(Name) virtual void Visit##Name(Name* node);
  FOR_EACH_CONCRETE_IR_NODE(V)
#undef V

 private:
  DISALLOW_COPY_AND_ASSIGN(Visitor);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_VISITOR_H_
