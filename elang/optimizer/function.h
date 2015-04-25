// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FUNCTION_H_
#define ELANG_OPTIMIZER_FUNCTION_H_

#include <ostream>

#include "base/basictypes.h"
#include "elang/base/zone_allocated.h"
#include "elang/optimizer/optimizer_export.h"

namespace elang {
namespace optimizer {

class EntryNode;
class ExitNode;
class FunctionType;
class Node;
class SequenceIdSource;
class Type;

//////////////////////////////////////////////////////////////////////
//
// Function
//
class ELANG_OPTIMIZER_EXPORT Function final : public ZoneAllocated {
 public:
  EntryNode* entry_node() const { return entry_node_; }
  ExitNode* exit_node() const { return exit_node_; }
  FunctionType* function_type() const { return function_type_; }
  size_t id() const { return id_; }
  size_t max_node_id() const;
  Type* parameters_type() const;
  Type* return_type() const;

 private:
  friend class Factory;

  explicit Function(SequenceIdSource* node_id_source,
                    FunctionType* function_type,
                    EntryNode* entry_node,
                    ExitNode* exit_node);

  EntryNode* const entry_node_;
  FunctionType* function_type_;
  ExitNode* const exit_node_;
  size_t id_;
  SequenceIdSource* const node_id_source_;

  DISALLOW_COPY_AND_ASSIGN(Function);
};

ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Function* function);
ELANG_OPTIMIZER_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                                const Function& function);

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FUNCTION_H_
