// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODES_H_
#define ELANG_OPTIMIZER_NODES_H_

#include <array>

#include "base/basictypes.h"
#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/float_types.h"
#include "elang/base/zone_deque.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/thing.h"

namespace elang {
class AtomicString;
namespace optimizer {

class Editor;
class FunctionType;
class Type;

#define DECLARE_OPTIMIZER_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);            \
  friend class Editor;                            \
  friend class NodeCache;                         \
  friend class NodeFactory;                       \
                                                  \
 protected:                                       \
  ~self() override = default;

#define DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(self, super) \
  DECLARE_OPTIMIZER_NODE_CLASS(self, super)

#define DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(self, super) \
  DECLARE_OPTIMIZER_NODE_CLASS(self, super);               \
                                                           \
 private:                                                  \
  Opcode opcode() const final;                             \
  void Accept(NodeVisitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// FloatCondition
// Make |CommuteCondition()| to simple, condition ^ 15, we assign constant
// to each condition.
//
#define FOR_EACH_OPTIMIZER_FLOAT_CONDITION(V) \
  V(OrderedEqual, "eq", 0)                    \
  V(OrderedGreaterThanOrEqual, "ge", 1)       \
  V(OrderedGreaterThan, "gt", 2)              \
  V(UnorderedGreaterThanOrEqual, "uge", 3)    \
  V(UnorderedGreaterThan, "ugt", 4)           \
  V(UnorderedEqual, "ueq", 5)                 \
  V(Invalid6, "invalid6", 6)                  \
  V(Invalid7, "invalid7", 7)                  \
  V(Invalid8, "invalid8", 8)                  \
  V(Invalid9, "invalid9", 9)                  \
  V(UnorderedNotEqual, "une", 10)             \
  V(UnorderedLessThanOrEqual, "ule", 11)      \
  V(UnorderedLessThan, "ult", 12)             \
  V(OrderedLessThanOrEqual, "le", 13)         \
  V(OrderedLessThan, "lt", 14)                \
  V(OrderedNotEqual, "ne", 15)

enum class FloatCondition {
#define V(Name, mnemonic, value) Name = value,
  FOR_EACH_OPTIMIZER_FLOAT_CONDITION(V)
#undef V
};

inline FloatCondition CommuteCondition(FloatCondition condition) {
  return static_cast<FloatCondition>(static_cast<int>(condition) ^ 15);
}

//////////////////////////////////////////////////////////////////////
//
// IntCondition
// Make |CommuteCondition()| to simple, condition ^ 15, we assign constant
// to each condition.
//
#define FOR_EACH_OPTIMIZER_INTEGER_CONDITION(V) \
  V(Equal, "eq", 0)                             \
  V(SignedGreaterThanOrEqual, "ge", 1)          \
  V(SignedGreaterThan, "gt", 2)                 \
  V(UnsignedGreaterThanOrEqual, "uge", 3)       \
  V(UnsignedGreaterThan, "ugt", 4)              \
  V(Invalid5, "invalid5", 5)                    \
  V(Invalid6, "invalid6", 6)                    \
  V(Invalid7, "invalid7", 7)                    \
  V(Invalid8, "invalid8", 8)                    \
  V(Invalid9, "invalid9", 9)                    \
  V(Invalid10, "invalid10", 10)                 \
  V(UnsignedLessThanOrEqual, "ule", 11)         \
  V(UnsignedLessThan, "ult", 12)                \
  V(SignedLessThanOrEqual, "le", 13)            \
  V(SignedLessThan, "lt", 14)                   \
  V(NotEqual, "ne", 15)

enum class IntCondition {
#define V(Name, mnemonic, value) Name = value,
  FOR_EACH_OPTIMIZER_INTEGER_CONDITION(V)
#undef V
};

inline IntCondition CommuteCondition(IntCondition condition) {
  return static_cast<IntCondition>(static_cast<int>(condition) ^ 15);
}

//////////////////////////////////////////////////////////////////////
//
// Input
//
class ELANG_OPTIMIZER_EXPORT Input final
    : public DoubleLinked<Input, Node>::Node {
 public:
  typedef ::elang::optimizer::Node Node;

  Input();
  ~Input() = default;

  Node* owner() const { return owner_; }
  Node* value() const { return value_; }

 protected:
  void Init(Node* owner, Node* value);
  void Reset();
  void SetValue(Node* new_value);

 private:
  friend class Editor;

  // |Node| calls |Init| during construction of |Node|.
  friend class Node;

  Node* value_;

  // Owner of this |Input| using |value_|.
  Node* owner_;

  DISALLOW_COPY_AND_ASSIGN(Input);
};

//////////////////////////////////////////////////////////////////////
//
// InputHolder
//
class InputHolder : public ZoneAllocated {
 public:
  InputHolder();
  ~InputHolder();

  Input* input() { return &input_; }

 private:
  Input input_;

  DISALLOW_COPY_AND_ASSIGN(InputHolder);
};

//////////////////////////////////////////////////////////////////////
//
// Opcode
//
enum class Opcode {
#define V(Name, ...) Name,
  FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V
      NumberOfOpcodes,
};

//////////////////////////////////////////////////////////////////////
//
// NodeLayout
//
class NodeLayout {
 public:
  virtual size_t field() const = 0;
  virtual bool has_field() const = 0;

  // Append input.
  virtual void AppendInput(Node* value) = 0;

  // Number of input operands.
  virtual size_t CountInputs() const = 0;

  // Access to |Input|
  virtual Input* InputAt(size_t index) const = 0;

  // Is variadic?
  virtual bool IsVariadic() const = 0;

 protected:
  NodeLayout();
  virtual ~NodeLayout();

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeLayout);
};

//////////////////////////////////////////////////////////////////////
//
// Node
//
class ELANG_OPTIMIZER_EXPORT Node : public Thing, public NodeLayout {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Node, Thing);

 public:
  // InputIterator
  class ELANG_OPTIMIZER_EXPORT InputIterator final {
   public:
    typedef std::forward_iterator_tag iterator_category;
    typedef int difference_type;
    typedef Node value_type;
    typedef Node* pointer;
    typedef Node& reference;

    InputIterator(const Node* node, size_t current);
    InputIterator(const InputIterator& other);
    ~InputIterator();

    InputIterator& operator=(const InputIterator& other);
    InputIterator& operator++();
    Node* operator*() const;
    Node* operator->() const;
    bool operator==(const InputIterator& other) const;
    bool operator!=(const InputIterator& other) const;

   private:
    const Node* node_;
    size_t current_;
  };

  // Inputs
  class ELANG_OPTIMIZER_EXPORT Inputs final {
   public:
    explicit Inputs(const Node* node);
    Inputs(const Inputs& other);
    ~Inputs();

    Inputs& operator=(const Inputs& other);

    InputIterator begin();
    InputIterator end();

   private:
    const Node* node_;
  };

  typedef DoubleLinked<Input, Node> Users;

  // A mnemonic string pf this node used in printer.
  virtual base::StringPiece mnemonic() const;

  // An operation code of this node.
  virtual Opcode opcode() const = 0;

  // An output type of this node.
  Type* output_type() const { return output_type_; }

  // Unique identifier of this |Node|, which is used for index to vector of
  // out-of-line data.
  size_t id() const { return id_; }

  // A value of |index|th input operand.
  Node* input(size_t index) const;
  Inputs inputs() const;
  const Users& users() const { return use_def_list_; }

  // Visitor pattern
  virtual void Accept(NodeVisitor* visitor) = 0;

  bool IsControl() const;
  bool IsData() const;
  bool IsEffect() const;
  bool IsLiteral() const;
  bool IsValidControl() const;
  bool IsValidData() const;
  bool IsValidEffect() const;
  bool IsValidEffectAt(size_t field) const;

  // NodeLayout
  size_t field() const override;
  bool has_field() const override;

 protected:
  // For calling |Use()| and |Unuse()|.
  friend class Input;

  explicit Node(Type* output_type);

  void set_id(size_t id);

  void InitInputAt(size_t index, Node* value);
  void ResetInputAt(size_t index);
  void SetInputAt(size_t index, Node* new_value);
  void Unuse(Input* input);
  void Use(Input* input);

  // NodeLayout
  void AppendInput(Node* value) override;
  Input* InputAt(size_t index) const override;
  bool IsVariadic() const override;

 private:
  uint32_t id_;
  Type* output_type_;
  Users use_def_list_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

//////////////////////////////////////////////////////////////////////
//
// Node template
//
template <size_t kNumberOfInputs, typename Base = Node>
class NodeTemplate : public Base {
 protected:
  explicit NodeTemplate(Type* output_type) : Base(output_type) {}
  ~NodeTemplate() override = default;

 private:
  // NodeLayout protocol
  size_t CountInputs() const final { return kNumberOfInputs; }

  Input* InputAt(size_t index) const {
    DCHECK_LT(index, kNumberOfInputs);
    return const_cast<Input*>(&inputs_[index]);
  }

  std::array<Input, kNumberOfInputs> inputs_;

  DISALLOW_COPY_AND_ASSIGN(NodeTemplate);
};

template <>
class NodeTemplate<0> : public Node {
 protected:
  explicit NodeTemplate(Type* output_type) : Node(output_type) {}

 private:
  // NodeLayout protocol
  size_t CountInputs() const final { return 0; }
  Input* InputAt(size_t index) const final {
    NOTREACHED();
    return nullptr;
  }

  DISALLOW_COPY_AND_ASSIGN(NodeTemplate);
};

//////////////////////////////////////////////////////////////////////
//
// FieldNodeTemplate
//
template <typename Base>
class FieldNodeTemplate : public NodeTemplate<1, Base> {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(FieldNodeTemplate, Base);

 public:
  // NodeLayout
  size_t field() const final { return field_; }
  bool has_field() const final { return true; }

 protected:
  FieldNodeTemplate(Type* output_type, Node* input, size_t field);

 private:
  size_t field_;

  DISALLOW_COPY_AND_ASSIGN(FieldNodeTemplate);
};

template <typename Base>
FieldNodeTemplate<Base>::FieldNodeTemplate(Type* output_type,
                                           Node* input, size_t field)
    : NodeTemplate(output_type), field_(field) {
  InitInputAt(0, input);
}

//////////////////////////////////////////////////////////////////////
//
// VariadicNodeTemplate
//
template <typename Base>
class VariadicNodeTemplate : public Base {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(VariadicNodeTemplate, Base);

 protected:
  VariadicNodeTemplate(Type* output_type, Zone* zone);

 private:
  // Node input protocol
  void AppendInput(Node* node) final;
  size_t CountInputs() const final { return inputs_.size(); }
  Input* InputAt(size_t index) const final;
  bool IsVariadic() const final { return true; }

  ZoneDeque<InputHolder*> inputs_;

  DISALLOW_COPY_AND_ASSIGN(VariadicNodeTemplate);
};

template <typename Base>
VariadicNodeTemplate<Base>::VariadicNodeTemplate(Type* output_type, Zone* zone)
    : Node(output_type), inputs_(zone) {
}

template <typename Base>
void VariadicNodeTemplate<Base>::AppendInput(Node* value) {
  auto const zone = inputs_.get_allocator().zone();
  inputs_.push_back(new (zone) InputHolder());
  InitInputAt(inputs_.size() - 1, value);
}

template <typename Base>
Input* VariadicNodeTemplate<Base>::InputAt(size_t index) const {
  return inputs_[index]->input();
}

//////////////////////////////////////////////////////////////////////
//
// PhiOwnerNode
//
class ELANG_OPTIMIZER_EXPORT PhiOwnerNode : public VariadicNodeTemplate<Node> {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(PhiOwnerNode, Node);

 public:
  typedef DoubleLinked<PhiNode, PhiOwnerNode> Phis;

  EffectPhiNode* effect_phi() const { return effect_phi_; }
  void set_effect_phi(EffectPhiNode* effect_phi);
  const Phis& phi_nodes() const { return phi_nodes_; }

 protected:
  PhiOwnerNode(Type* output_type, Zone* zone);

 private:
  EffectPhiNode* effect_phi_;
  Phis phi_nodes_;

  DISALLOW_COPY_AND_ASSIGN(PhiOwnerNode);
};

// Concrete classes

//////////////////////////////////////////////////////////////////////
//
// Literals
//
#define V(Name, mnemonic, data_type)                                       \
  class ELANG_OPTIMIZER_EXPORT Name##Node final : public NodeTemplate<0> { \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Node);               \
                                                                           \
   public:                                                                 \
    data_type data() const { return data_; }                               \
                                                                           \
   private:                                                                \
    Name##Node(Type* output_type, data_type data);                         \
                                                                           \
    data_type const data_;                                                 \
                                                                           \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                  \
  };
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Simple nodes
//
#define V(Name, ...)                                                       \
  class ELANG_OPTIMIZER_EXPORT Name##Node final : public NodeTemplate<1> { \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Node);               \
                                                                           \
   private:                                                                \
    Name##Node(Type* output_type, Node* input0);                           \
                                                                           \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                  \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V)
#undef V

#define V(Name, ...)                                                       \
  class ELANG_OPTIMIZER_EXPORT Name##Node final : public NodeTemplate<2> { \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Node);               \
                                                                           \
   private:                                                                \
    Name##Node(Type* output_type, Node* input0, Node* input1);             \
                                                                           \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                  \
  };
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V)
#undef V

#define V(Name, ...)                                                         \
  class ELANG_OPTIMIZER_EXPORT Name##Node final : public NodeTemplate<3> {   \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Node);                 \
                                                                             \
   private:                                                                  \
    Name##Node(Type* output_type, Node* input0, Node* input1, Node* input2); \
                                                                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                    \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V)
#undef V

#define V(Name, ...)                                         \
  class ELANG_OPTIMIZER_EXPORT Name##Node final              \
      : public VariadicNodeTemplate<Node> {                  \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Node); \
                                                             \
   private:                                                  \
    Name##Node(Type* output_type, Zone* zone);               \
                                                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                    \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_V(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// EffectGetNode
//
class ELANG_OPTIMIZER_EXPORT EffectGetNode final
    : public FieldNodeTemplate<Effect> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(EffectGetNode, FieldNodeTemplate);

 private:
  EffectGetNode(Type* effect_type, Node* input, size_t field);

  DISALLOW_COPY_AND_ASSIGN(EffectGetNode);
};

//////////////////////////////////////////////////////////////////////
//
// EntryNode
//
class ELANG_OPTIMIZER_EXPORT EntryNode final : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(EntryNode, Node);

 public:
  Type* parameters_type() const;
  Type* parameter_type(size_t index) const;

  Type* CheckedParameterTypeAt(size_t index) const;

 private:
  explicit EntryNode(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(EntryNode);
};

//////////////////////////////////////////////////////////////////////
//
// FloatCmpNode
//
class ELANG_OPTIMIZER_EXPORT FloatCmpNode final : public NodeTemplate<2> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(FloatCmpNode, Node);

 public:
  FloatCondition condition() const { return condition_; }

 private:
  FloatCmpNode(Type* output_type,
               FloatCondition condition,
               Node* left,
               Node* right);

  base::StringPiece mnemonic() const final;

  FloatCondition condition_;

  DISALLOW_COPY_AND_ASSIGN(FloatCmpNode);
};

//////////////////////////////////////////////////////////////////////
//
// FunctionReferenceNode
//
class ELANG_OPTIMIZER_EXPORT FunctionReferenceNode final
    : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(FunctionReferenceNode, Node);

 public:
  Function* function() const { return function_; }

 private:
  FunctionReferenceNode(Type* output_type, Function* function);

  Function* const function_;

  DISALLOW_COPY_AND_ASSIGN(FunctionReferenceNode);
};

//////////////////////////////////////////////////////////////////////
//
// GetNode
//
class ELANG_OPTIMIZER_EXPORT GetNode final : public FieldNodeTemplate<Node> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(GetNode, FieldNodeTemplate);

 private:
  GetNode(Type* output_type, Node* input, size_t field);

  DISALLOW_COPY_AND_ASSIGN(GetNode);
};

//////////////////////////////////////////////////////////////////////
//
// IntCmpNode
//
class ELANG_OPTIMIZER_EXPORT IntCmpNode final : public NodeTemplate<2> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(IntCmpNode, Node);

 public:
  IntCondition condition() const { return condition_; }

 private:
  IntCmpNode(Type* output_type,
             IntCondition condition,
             Node* left,
             Node* right);

  base::StringPiece mnemonic() const final;

  IntCondition condition_;

  DISALLOW_COPY_AND_ASSIGN(IntCmpNode);
};

//////////////////////////////////////////////////////////////////////
//
// LoopNode
//
class ELANG_OPTIMIZER_EXPORT LoopNode final : public PhiOwnerNode {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(LoopNode, PhiOwnerNode);

 private:
  LoopNode(Type* output_type, Zone* zone);

  DISALLOW_COPY_AND_ASSIGN(LoopNode);
};

//////////////////////////////////////////////////////////////////////
//
// MergeNode
//
class ELANG_OPTIMIZER_EXPORT MergeNode final : public PhiOwnerNode {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(MergeNode, PhiOwnerNode);

 private:
  MergeNode(Type* output_type, Zone* zone);

  DISALLOW_COPY_AND_ASSIGN(MergeNode);
};

//////////////////////////////////////////////////////////////////////
//
// NullNode
//
class ELANG_OPTIMIZER_EXPORT NullNode final : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(NullNode, Node);

 private:
  explicit NullNode(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(NullNode);
};

//////////////////////////////////////////////////////////////////////
//
// |ParameterNode| extracts a data parameter at |field| from |EntryNode|.
// Unique parameter nodes are appeared at most once in |Function|. Parameter
// nodes must be scheduled followed by |EntryNode| continuously.
//
class ELANG_OPTIMIZER_EXPORT ParameterNode final
    : public FieldNodeTemplate<Node> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(ParameterNode, FieldNodeTemplate);

 private:
  ParameterNode(Type* output_type, Node* input, size_t field);

  DISALLOW_COPY_AND_ASSIGN(ParameterNode);
};

//////////////////////////////////////////////////////////////////////
//
// PhiNode
//
class ELANG_OPTIMIZER_EXPORT PhiNode final
    : public VariadicNodeTemplate<Node>,
      public DoubleLinked<PhiNode, PhiOwnerNode>::Node {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(PhiNode, Node);

 public:
  typedef ::elang::optimizer::Node Node;

  PhiOwnerNode* owner() const { return owner_; }

 private:
  PhiNode(Type* output_type, Zone* zone, PhiOwnerNode* control);

  PhiOwnerNode* owner_;

  DISALLOW_COPY_AND_ASSIGN(PhiNode);
};

//////////////////////////////////////////////////////////////////////
//
// EffectPhiNode
//
class ELANG_OPTIMIZER_EXPORT EffectPhiNode final
    : public VariadicNodeTemplate<Node> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(EffectPhiNode, Node);

 public:
  PhiOwnerNode* owner() const { return owner_; }

 private:
  EffectPhiNode(Type* output_type, Zone* zone, PhiOwnerNode* control);

  PhiOwnerNode* owner_;

  DISALLOW_COPY_AND_ASSIGN(EffectPhiNode);
};

//////////////////////////////////////////////////////////////////////
//
// ReferenceNode
//
class ELANG_OPTIMIZER_EXPORT ReferenceNode final : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(ReferenceNode, Node);

 public:
  AtomicString* name() const { return name_; }

 private:
  ReferenceNode(Type* output_type, AtomicString* name);

  AtomicString* const name_;

  DISALLOW_COPY_AND_ASSIGN(ReferenceNode);
};

//////////////////////////////////////////////////////////////////////
//
// SizeOfNode
//
class ELANG_OPTIMIZER_EXPORT SizeOfNode final : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(SizeOfNode, Node);

 public:
  Type* type_operand() const { return type_operand_; }

 private:
  SizeOfNode(Type* uintptr_type, Type* type_operand);

  Type* const type_operand_;

  DISALLOW_COPY_AND_ASSIGN(SizeOfNode);
};

//////////////////////////////////////////////////////////////////////
//
// StoreNode - effect %new_effect = store %effect, %base, %pointer, %value
//
class ELANG_OPTIMIZER_EXPORT StoreNode final : public NodeTemplate<4, Effect> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(StoreNode, Effect);

 private:
  StoreNode(Type* effect_type, Node* input, size_t field);

  DISALLOW_COPY_AND_ASSIGN(StoreNode);
};

//////////////////////////////////////////////////////////////////////
//
// VoidNode
//
class ELANG_OPTIMIZER_EXPORT VoidNode final : public NodeTemplate<0> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(VoidNode, Node);

 private:
  explicit VoidNode(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(VoidNode);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODES_H_
