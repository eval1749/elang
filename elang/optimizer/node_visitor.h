// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODE_VISITOR_H_
#define ELANG_OPTIMIZER_NODE_VISITOR_H_

#include "base/logging.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/nodes_forward.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeVisitor
//
class ELANG_OPTIMIZER_EXPORT NodeVisitor {
 public:
#define V(Name, ...) virtual void Visit##Name(Name##Node* node);
  FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

 protected:
  NodeVisitor();
  virtual ~NodeVisitor();

  virtual void DoDefaultVisit(Node* node);

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeVisitor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_VISITOR_H_
