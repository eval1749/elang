// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_OPERAND_VISITOR_H_
#define ELANG_HIR_OPERAND_VISITOR_H_

#include "base/logging.h"
#include "elang/hir/operands_forward.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// OperandVisitor
//
class OperandVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name* type) = 0;
  FOR_EACH_HIR_OPERAND(V)
#undef V

 protected:
  OperandVisitor();
  virtual ~OperandVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(OperandVisitor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_OPERAND_VISITOR_H_
