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
  Control* NewControlGet(Node* input, size_t field);
  Node* NewDynamicCast(Type* output_type, Node* input);
  Effect* NewEffectGet(Node* input, size_t field);
  Node* NewFunctionReference(Function* function);
  Node* NewGet(Node* input, size_t field);
  Control* NewIfFalse(Control* input);
  Control* NewIfSuccess(Control* input);
  Control* NewIfTrue(Control* input);
  Control* NewJump(Control* input);
  Node* NewStaticCast(Type* output_type, Node* input);
  Control* NewSwitch(Control* control, Node* value);
  Control* NewUnreachable(Control* input);

  // Two inputs
  Node* NewFloatCmp(FloatCondition condition, Node* left, Node* right);
  Control* NewIf(Control* control, Node* value);
  Node* NewIntCmp(IntCondition condition, Node* left, Node* right);
  Node* NewIntShl(Node* left, Node* right);
  Node* NewIntShr(Node* left, Node* right);
  Node* NewParameter(Node* input0, size_t field);
  Node* NewPhiOperand(Control* control, Node* value);
  Node* NewThrow(Control* control, Node* value);

  // Three inputs
  Node* NewCall(Effect* effect, Node* callee, Node* arguments);
  Node* NewLoad(Effect* effect, Node* base_pointer, Node* pointer);
  Control* NewRet(Control* control, Effect* effect, Node* data);

  // Four inputs
  Effect* NewStore(Effect* effect,
                   Node* base_pointer,
                   Node* pointer,
                   Node* value);

  // Variadic inputs
  Node* NewCase(Control* control, Node* label_value);
  EffectPhiNode* NewEffectPhi(PhiOwnerNode* owner);
  Node* NewLoop(Control* control);
  PhiOwnerNode* NewMerge(const std::vector<Control*>& inputs);
  PhiNode* NewPhi(Type* type, PhiOwnerNode* owner);
  Node* NewTuple(const std::vector<Node*>& inputs);
  Node* NewTuple(Type* type);

 private:
  // For using |NewEntry()| and |NewExit()|.
  friend class Factory;

  SequenceIdSource* node_id_source() const;

  // Node cache management
  Node* FindBinaryNode(Opcode opcode, Node* left, Node* right);
  Node* FindFieldNode(Node* input, size_t field);
  Node* FindUnaryNode(Opcode opcode, Type* type, Node* input);
  void RememberBinaryNode(Node* node);
  void RememberFieldNode(Node* node, Node* input, size_t field);
  void RememberUnaryNode(Node* node);

  Node* NewEntry(Type* parameters_type);
  Node* NewExit(Control* control);

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
