// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LITERAL_VISITOR_H_
#define ELANG_LIR_LITERAL_VISITOR_H_

#include "base/logging.h"
#include "elang/lir/literals_forward.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// LiteralVisitor
//
class LiteralVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name* type) = 0;
  FOR_EACH_LIR_LITERAL(V)
#undef V

 protected:
  LiteralVisitor();
  virtual ~LiteralVisitor();

 private:
  DISALLOW_COPY_AND_ASSIGN(LiteralVisitor);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_LITERAL_VISITOR_H_
