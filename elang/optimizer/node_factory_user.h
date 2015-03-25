// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODE_FACTORY_USER_H_
#define ELANG_OPTIMIZER_NODE_FACTORY_USER_H_

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/nodes_forward.h"

namespace elang {
class AtomicString;

namespace optimizer {

class FunctionType;

//////////////////////////////////////////////////////////////////////
//
// NodeFactoryUser
//
class ELANG_OPTIMIZER_EXPORT NodeFactoryUser {
 public:
  ~NodeFactoryUser();

  Node* false_value() const;
  NodeFactory* node_factory() const { return node_factory_; }
  Node* true_value() const;

// Literal nodes
#define V(Name, mnemonic, data_type) Node* New##Name(data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

 protected:
  explicit NodeFactoryUser(NodeFactory* node_factory);

 private:
  NodeFactory* const node_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactoryUser);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_FACTORY_USER_H_
