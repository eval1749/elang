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

  Data* false_value() const { return false_value_; }
  Data* true_value() const { return true_value_; }
  Data* void_value() const { return void_value_; }

  Data* DefaultValueOf(Type* type);
  Data* NewNull(Type* type);
  Data* NewSizeOf(Type* type);

// Arithmetic nodes
#define V(Name, ...) Data* New##Name(Data* input0, Data* input1);
  FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
#undef V

  // Literal nodes
  Data* NewReference(Type* type, AtomicString* name);
#define V(Name, mnemonic, data_type) Data* New##Name(data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

  // Single input
  Data* NewDynamicCast(Type* output_type, Data* input);
  Data* NewFunctionReference(Function* function);
  Data* NewGet(Tuple* input, size_t field);
  Data* NewGetData(Control* input);
  Effect* NewGetEffect(Control* input);
  Tuple* NewGetTuple(Control* input);
  Control* NewIfFalse(Control* input);
  Control* NewIfSuccess(Control* input);
  Control* NewIfTrue(Control* input);
  Control* NewJump(Control* input);
  Data* NewStaticCast(Type* output_type, Data* input);
  Control* NewSwitch(Control* control, Data* value);
  Control* NewUnreachable(Control* input);

  // Two inputs
  Data* NewElement(Data* array, Node* indexes);
  Data* NewFloatCmp(FloatCondition condition, Data* left, Data* right);
  Control* NewIf(Control* control, Data* value);
  Data* NewIntCmp(IntCondition condition, Data* left, Data* right);
  Data* NewIntShl(Data* left, Data* right);
  Data* NewIntShr(Data* left, Data* right);
  Data* NewLength(Data* array, size_t rank);
  Data* NewParameter(EntryNode* entry_node, size_t field);
  Data* NewThrow(Control* control, Data* value);

  // Three inputs
  Data* NewLoad(Effect* effect, Data* base_pointer, Data* pointer);
  Control* NewRet(Control* control, Effect* effect, Data* data);

  // Four inputs
  Control* NewCall(Control* control,
                   Effect* effect,
                   Data* callee,
                   Node* arguments);
  Effect* NewStore(Effect* effect,
                   Data* base_pointer,
                   Data* pointer,
                   Data* value);

  // Variadic inputs
  Data* NewCase(Control* control, Data* label_value);
  EffectPhiNode* NewEffectPhi(PhiOwnerNode* owner);
  LoopNode* NewLoop();
  PhiOwnerNode* NewMerge(const std::vector<Control*>& inputs);
  PhiNode* NewPhi(Type* type, PhiOwnerNode* owner);
  Tuple* NewTuple(const std::vector<Node*>& inputs);

 private:
  // For using |NewEntry()| and |NewExit()|.
  friend class Factory;

  SequenceIdSource* node_id_source() const;

  EntryNode* NewEntry(Type* parameters_type);
  ExitNode* NewExit(Control* control);

  size_t NewNodeId();

  const std::unique_ptr<NodeCache> node_cache_;

  // |false_value_| and |true_value| depend on |node_cache_|.
  Data* const false_value_;
  Data* const true_value_;
  Data* const void_value_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_FACTORY_H_
