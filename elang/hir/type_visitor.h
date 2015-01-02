// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPE_VISITOR_H_
#define ELANG_HIR_TYPE_VISITOR_H_

#include "base/logging.h"
#include "elang/hir/types_forward.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// TypeVisitor
//
class TypeVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name##Type(Name##Type* type) = 0;
  FOR_EACH_HIR_TYPE(V)
#undef V

 protected:
  TypeVisitor();
  virtual ~TypeVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(TypeVisitor);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPE_VISITOR_H_
