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

  Node* false_value() const;
  NodeFactory* node_factory() const { return node_factory_; }
  Node* true_value() const;
  Node* void_value() const;

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
  Control* NewControlGet(Node* input, size_t field);
  Effect* NewEffectGet(Node* input, size_t field);
  Node* NewDynamicCast(Type* output_type, Node* input);
  Node* NewGet(Node* input, size_t field);
  Node* NewFunctionReference(Function* function);
  Control* NewIfFalse(Control* control);
  Control* NewIfSuccess(Control* control);
  Control* NewIfTrue(Control* control);
  Control* NewJump(Control* control);
  Node* NewStaticCast(Type* output_type, Node* input);
  Control* NewUnreachable(Control* control);

  // Two inputs
  Node* NewFloatCmp(FloatCondition condition, Node* left, Node* right);
  Control* NewIf(Control* control, Node* value);
  Node* NewIntCmp(IntCondition condition, Node* left, Node* right);
  Node* NewIntShl(Node* left, Node* right);
  Node* NewIntShr(Node* left, Node* right);
  Node* NewParameter(Node* input0, size_t field);
  Control* NewSwitch(Control* control, Node* value);
  Node* NewThrow(Control* control, Node* value);

  // Three inputs
  Node* NewCall(Effect* effect, Node* callee, Node* arguments);
  Node* NewLoad(Effect* effect, Node* base_pointer, Node* pointer);
  Control* NewRet(Control* control, Effect* effect, Node* value);

  // Four inputs
  Effect* NewStore(Effect* effect,
                   Node* base_pointer,
                   Node* pointer,
                   Node* value);

  // Variable inputs
  Node* NewCase(Control* control, Node* label_value);
  EffectPhiNode* NewEffectPhi(PhiOwnerNode* owner);
  Node* NewLoop(Control* control);
  PhiOwnerNode* NewMerge(const std::vector<Control*>& inputs);
  PhiNode* NewPhi(Type* type, PhiOwnerNode* owner);
  Node* NewTuple(const std::vector<Node*>& inputs);
  Node* NewTuple(Type* type);

 protected:
  explicit NodeFactoryUser(NodeFactory* node_factory);

 private:
  NodeFactory* const node_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeFactoryUser);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_FACTORY_USER_H_
