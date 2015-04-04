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
class Type;

//////////////////////////////////////////////////////////////////////
//
// NodeFactoryUser
//
class ELANG_OPTIMIZER_EXPORT NodeFactoryUser {
 public:
  ~NodeFactoryUser();

  Data* false_value() const;
  NodeFactory* node_factory() const { return node_factory_; }
  Data* true_value() const;
  Data* void_value() const;

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
  Control* NewControlGet(Tuple* input, size_t field);
  Effect* NewEffectGet(Tuple* input, size_t field);
  Data* NewDynamicCast(Type* output_type, Data* input);
  Data* NewGet(Tuple* input, size_t field);
  Data* NewFunctionReference(Function* function);
  Control* NewIfFalse(Control* control);
  Control* NewIfSuccess(Control* control);
  Control* NewIfTrue(Control* control);
  Control* NewJump(Control* control);
  Data* NewStaticCast(Type* output_type, Data* input);
  Control* NewUnreachable(Control* control);

  // Two inputs
  Data* NewElement(Data* array, Node* indexes);
  Data* NewFloatCmp(FloatCondition condition, Data* left, Data* right);
  Control* NewIf(Control* control, Data* value);
  Data* NewIntCmp(IntCondition condition, Data* left, Data* right);
  Data* NewIntShl(Data* left, Data* right);
  Data* NewIntShr(Data* left, Data* right);
  Data* NewLength(Data* array, size_t rank);
  Data* NewParameter(EntryNode* entry_node, size_t field);
  Control* NewSwitch(Control* control, Data* value);
  Control* NewThrow(Control* control, Data* value);

  // Three inputs
  Data* NewLoad(Effect* effect, Data* base_pointer, Data* pointer);
  Control* NewRet(Control* control, Effect* effect, Data* value);

  // Four inputs
  Tuple* NewCall(Control* control,
                 Effect* effect,
                 Data* callee,
                 Node* arguments);
  Effect* NewStore(Effect* effect,
                   Node* base_pointer,
                   Node* pointer,
                   Node* value);

  // Variable inputs
  Control* NewCase(Control* control, Data* label);
  EffectPhiNode* NewEffectPhi(PhiOwnerNode* owner);
  Control* NewLoop();
  PhiOwnerNode* NewMerge(const std::vector<Control*>& inputs);
  PhiNode* NewPhi(Type* type, PhiOwnerNode* owner);
  Tuple* NewTuple(const std::vector<Node*>& inputs);

 protected:
  explicit NodeFactoryUser(NodeFactory* node_factory);

 private:
  NodeFactory* const node_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactoryUser);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_FACTORY_USER_H_
