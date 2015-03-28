// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODE_FACTORY_H_
#define ELANG_OPTIMIZER_NODE_FACTORY_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/types_forward.h"
#include "elang/optimizer/type_factory_user.h"

namespace elang {
namespace optimizer {

class NodeCache;
class SequenceIdSource;

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
class ELANG_OPTIMIZER_EXPORT NodeFactory final : public TypeFactoryUser,
                                                 public ZoneOwner {
 public:
  explicit NodeFactory(TypeFactory* type_factory);
  ~NodeFactory();

  Node* false_value() const { return false_value_; }
  Node* true_value() const { return true_value_; }
  Node* void_value() const { return void_value_; }

  Node* DefaultValueOf(Type* type);
  Node* NewNull(Type* type);

// Arithmetic nodes
#define V(Name, ...) Node* New##Name(Node* input0, Node* input1);
  FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
#undef V

  // Literal nodes
  Node* NewReference(Type* type, AtomicString* name);
#define V(Name, mnemonic, data_type) Node* New##Name(data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

  // Single input
  Node* NewDynamicCast(Type* output_type, Node* input);
  Node* NewFunctionReference(Function* function);
  Node* NewIfFalse(Node* input);
  Node* NewIfSuccess(Node* input);
  Node* NewIfTrue(Node* input);
  Node* NewJump(Node* input);
  Node* NewStaticCast(Type* output_type, Node* input);
  Node* NewSwitch(Node* input);
  Node* NewUnreachable(Node* input);

  // Two inputs
  Node* NewGet(Node* input0, size_t field);
  Node* NewIf(Node* control, Node* value);
  Node* NewParameter(Node* input0, size_t field);
  Node* NewPhiInput(Node* control, Node* value);
  Node* NewShl(Node* input0, Node* input1);
  Node* NewShr(Node* input0, Node* input1);
  Node* NewThrow(Node* control, Node* value);

  // Three inputs
  Node* NewLoad(Node* effect, Node* base_pointer, Node* pointer);
  Node* NewRet(Node* control, Node* effect, Node* data);

  // Four inputs
  Node* NewCall(Node* effect, Node* control, Node* callee, Node* arguments);
  Node* NewStore(Node* effect, Node* base_pointer, Node* pointer, Node* value);

  // Variable inputs
  Node* NewCase(Node* control, Node* label_value);
  Node* NewLoop(Node* control);
  Node* NewMerge(Node* control0, Node* control1);
  Node* NewPhi(Type* type);
  Node* NewTuple(Type* type);

 private:
  friend class Factory;
  class LiteralNodeCache;

  SequenceIdSource* node_id_source() const;

  Node* NewEntry(Type* parameters_type);
  Node* NewExit(Node* control);
  Node* NewMerge();

  size_t NewNodeId();

  const std::unique_ptr<NodeCache> node_cache_;

  // |false_value_| and |true_value| depend on |node_cache_|.
  Node* const false_value_;
  Node* const true_value_;
  Node* const void_value_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_FACTORY_H_
