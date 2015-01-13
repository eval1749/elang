// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_VALUE_VISITOR_H_
#define ELANG_HIR_VALUE_VISITOR_H_

#include "base/logging.h"
#include "elang/hir/values_forward.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// ValueVisitor
//
class ValueVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name* type) = 0;
  FOR_EACH_HIR_VALUE(V)
#undef V

 protected:
  ValueVisitor();
  virtual ~ValueVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(ValueVisitor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_VALUE_VISITOR_H_
