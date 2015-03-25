// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/function.h"

#include "elang/optimizer/sequence_id_source.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

// Function
Function::Function(SequenceIdSource* node_id_source,
                   FunctionType* function_type,
                   Node* entry_node,
                   Node* exit_node)
    : entry_node_(entry_node),
      function_type_(function_type),
      exit_node_(exit_node),
      node_id_source_(node_id_source) {
}

size_t Function::max_node_id() const {
  return node_id_source_->last_id();
}

Type* Function::parameters_type() const {
  return function_type()->parameters_type();
}

Type* Function::return_type() const {
  return function_type()->return_type();
}

}  // namespace optimizer
}  // namespace elang