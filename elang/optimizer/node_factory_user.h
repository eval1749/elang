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
  Effect* NewEffectGet(Node* input, size_t field);
  Node* NewDynamicCast(Type* output_type, Node* input);
  Node* NewGet(Node* input0, size_t field);
  Node* NewFunctionReference(Function* function);
  Node* NewIfFalse(Node* input);
  Node* NewIfSuccess(Node* input);
  Node* NewIfTrue(Node* input);
  Node* NewJump(Node* control);
  Node* NewStaticCast(Type* output_type, Node* input);
  Node* NewSwitch(Node* input);
  Node* NewUnreachable(Node* input);

  // Two inputs
  Node* NewFloatCmp(FloatCondition condition, Node* left, Node* right);
  Node* NewIf(Node* control, Node* value);
  Node* NewIntCmp(IntCondition condition, Node* left, Node* right);
  Node* NewIntShl(Node* left, Node* right);
  Node* NewIntShr(Node* left, Node* right);
  Node* NewParameter(Node* input0, size_t field);
  Node* NewPhiOperand(Node* control, Node* value);
  Node* NewThrow(Node* control, Node* value);

  // Three inputs
  Node* NewCall(Node* effect, Node* callee, Node* arguments);
  Node* NewLoad(Node* effect, Node* base_pointer, Node* pointer);
  Node* NewRet(Node* control, Node* effect, Node* value);

  // Four inputs
  Node* NewStore(Node* effect, Node* base_pointer, Node* pointer, Node* value);

  // Variable inputs
  Node* NewCase(Node* control, Node* label_value);
  Effect* NewEffectPhi(Node* owner);
  Node* NewLoop(Node* control);
  Node* NewMerge(const std::vector<Node*>& inputs);
  Node* NewPhi(Type* type, Node* owner);
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
