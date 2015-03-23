// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_TYPE_VISITOR_H_
#define ELANG_OPTIMIZER_TYPE_VISITOR_H_

#include "base/logging.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/types_forward.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// TypeVisitor
//
class ELANG_OPTIMIZER_EXPORT TypeVisitor {
 public:
#define V(Name) virtual void Visit##Name(Name* type) = 0;
  FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V)
#undef V

 protected:
  TypeVisitor();
  virtual ~TypeVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeVisitor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_TYPE_VISITOR_H_
