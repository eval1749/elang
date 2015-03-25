// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/type_visitor.h"

#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// TypeVisitor
//
TypeVisitor::TypeVisitor() {
}

TypeVisitor::~TypeVisitor() {
}

void TypeVisitor::DoDefaultVisit(Type* type) {
}

#define V(Name) \
  void TypeVisitor::Visit##Name(Name* type) { DoDefaultVisit(type); }
FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V)
#undef V

}  // namespace optimizer
}  // namespace elang
