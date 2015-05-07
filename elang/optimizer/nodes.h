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
#include "elang/base/work_list.h"
#include "elang/base/zone_deque.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/thing.h"

namespace elang {
class AtomicString;
// TODO(eval1749) Once our tool chain C++14 |std::is_final<T>|, we don't need
// to define |elang::is_final<T>| for |Node|s.
#define V(Name, ...) \
  template <>        \
  struct is_final<optimizer::Name##Node> : std::true_type {};
FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V
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
// Use edge; an edge from user to used. Instances are embedded into |Node|
// as input operands.
//
class ELANG_OPTIMIZER_EXPORT Input final
    : public DoubleLinked<Input, Node>::NodeBase {
 public:
  Input();
  ~Input() = default;

  Node* from() const { return from_; }
  Node* to() const { return to_; }

  // TODO(eval1749) We should replace |owner()| and |value()|.
  Node* owner() const { return from(); }
  Node* value() const { return to(); }

 protected:
  void Init(Node* from, Node* to);
  void Reset();
  void SetTo(Node* new_to);

 private:
  friend class Editor;

  // |Node| calls |Init| during construction of |Node|.
  friend class Node;

  Node* from_;  // user of |to_|.
  Node* to_;    // used by |from_|

  DISALLOW_COPY_AND_ASSIGN(Input);
};

// TODO(eval1749) We should rename |Input| to |UseEdge|.
using UseEdge = Input;
using UseEdges = DoubleLinked<Input, Node>;

//////////////////////////////////////////////////////////////////////
//
// InputHolder
//
class InputHolder final : public ZoneAllocated {
 public:
  InputHolder();
  ~InputHolder();

  const Input* input() const { return &input_; }
  Input* input() { return &input_; }

 private:
  Input input_;

  DISALLOW_COPY_AND_ASSIGN(InputHolder);
};

//////////////////////////////////////////////////////////////////////
//
// PhiInputHolder
//
class PhiInputHolder final : public ZoneAllocated {
 public:
  ~PhiInputHolder();

  Control* control() const { return control_; }
  Input* input() { return &input_; }
  Node* value() const { return input_.value(); }

 private:
  friend class Editor;

  explicit PhiInputHolder(Control* control);

  Input input_;
  Control* control_;

  DISALLOW_COPY_AND_ASSIGN(PhiInputHolder);
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
// Since |Node| class doesn't provide modification functions, you can use
// |Editor| class to modify state of |Node| instances.
//
class ELANG_OPTIMIZER_EXPORT Node : public Thing,
                                    public NodeLayout,
                                    public WorkList<Node>::Item {
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
  Control* control(size_t index) const;
  Node* input(size_t index) const;
  Inputs inputs() const;
  const UseEdges& use_edges() const { return use_edges_; }

  // Visitor pattern
  virtual void Accept(NodeVisitor* visitor) = 0;

  bool IsBlockEnd() const;
  bool IsBlockLabel() const;
  bool IsBlockStart() const;
  virtual bool IsControl() const;
  bool IsControlEffect() const;
  virtual bool IsData() const;
  virtual bool IsEffect() const;
  virtual bool IsLiteral() const;
  virtual bool IsTuple() const;
  bool IsUsed() const;
  bool IsValidControl() const;
  bool IsValidControlAt(size_t field) const;
  bool IsValidData() const;
  bool IsValidEffect() const;
  bool IsValidEffectAt(size_t field) const;

  // Select a user
  Node* SelectUser(Opcode opcode) const;
  Node* SelectUserIfOne() const;

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
  UseEdges use_edges_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

//////////////////////////////////////////////////////////////////////
//
// Control
//
class ELANG_OPTIMIZER_EXPORT Control : public Node {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Control, Node);

 protected:
  explicit Control(Type* output_type);

 private:
  bool IsControl() const final;

  DISALLOW_COPY_AND_ASSIGN(Control);
};

//////////////////////////////////////////////////////////////////////
//
// Data
//
class ELANG_OPTIMIZER_EXPORT Data : public Node {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Data, Node);

 protected:
  explicit Data(Type* output_type);

 private:
  bool IsData() const final;

  DISALLOW_COPY_AND_ASSIGN(Data);
};

//////////////////////////////////////////////////////////////////////
//
// Effect
//
class ELANG_OPTIMIZER_EXPORT Effect : public Node {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Effect, Node);

 protected:
  explicit Effect(Type* output_type);

 private:
  bool IsEffect() const final;

  DISALLOW_COPY_AND_ASSIGN(Effect);
};

//////////////////////////////////////////////////////////////////////
//
// Literal
//
class ELANG_OPTIMIZER_EXPORT Literal : public Data {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Literal, Data);

 protected:
  explicit Literal(Type* output_type);

 private:
  bool IsLiteral() const final;

  DISALLOW_COPY_AND_ASSIGN(Literal);
};

//////////////////////////////////////////////////////////////////////
//
// Tuple
//
class ELANG_OPTIMIZER_EXPORT Tuple : public Node {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(Tuple, Node);

 protected:
  explicit Tuple(Type* output_type);

 private:
  bool IsData() const final;
  bool IsTuple() const final;

  DISALLOW_COPY_AND_ASSIGN(Tuple);
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

template <typename Base>
class NodeTemplate<0, Base> : public Base {
 protected:
  explicit NodeTemplate(Type* output_type) : Base(output_type) {}

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
// LiteralNodeTemplate
//
template <typename DataType>
class LiteralNodeTemplate final : public NodeTemplate<0, Literal> {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(LiteralNodeTemplate, Literal);

 public:
  DataType data() const { return data_; }

 protected:
  LiteralNodeTemplate(Type* output_type, DataType data)
      : NodeTemplate(output_type), data_(data) {}

 private:
  DataType const data_;

  DISALLOW_COPY_AND_ASSIGN(LiteralNodeTemplate);
};

//////////////////////////////////////////////////////////////////////
//
// PhiNodeTemplate
//
template <typename Base>
class PhiNodeTemplate : public Base {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(PhiNodeTemplate, Base);

 public:
  const ZoneDeque<PhiInputHolder*>& phi_inputs() const { return phi_inputs_; }
  PhiOwnerNode* owner() const { return owner_; }

 protected:
  PhiNodeTemplate(Type* output_type, Zone* zone, PhiOwnerNode* owner);

 private:
  // NodeLayout
  size_t CountInputs() const final { return phi_inputs_.size(); }
  Input* InputAt(size_t index) const final;
  bool IsVariadic() const final { return true; }

  PhiOwnerNode* owner_;
  ZoneDeque<PhiInputHolder*> phi_inputs_;

  DISALLOW_COPY_AND_ASSIGN(PhiNodeTemplate);
};

template <typename Base>
PhiNodeTemplate<Base>::PhiNodeTemplate(Type* output_type,
                                       Zone* zone,
                                       PhiOwnerNode* owner)
    : Base(output_type), owner_(owner), phi_inputs_(zone) {
}

template <typename Base>
Input* PhiNodeTemplate<Base>::InputAt(size_t index) const {
  return phi_inputs_[index]->input();
}

//////////////////////////////////////////////////////////////////////
//
// ProjectionNodeTemplate
//
template <typename Base>
class ProjectionNodeTemplate : public NodeTemplate<1, Base> {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(ProjectionNodeTemplate, Base);

 public:
  // NodeLayout
  size_t field() const final { return field_; }
  bool has_field() const final { return true; }

 protected:
  ProjectionNodeTemplate(Type* output_type, Tuple* input, size_t field);

 private:
  size_t field_;

  DISALLOW_COPY_AND_ASSIGN(ProjectionNodeTemplate);
};

template <typename Base>
ProjectionNodeTemplate<Base>::ProjectionNodeTemplate(Type* output_type,
                                                     Tuple* input,
                                                     size_t field)
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

 public:
  // NodeLayout
  void AppendInput(Node* node) final;
  size_t CountInputs() const final { return inputs_.size(); }

 protected:
  VariadicNodeTemplate(Type* output_type, Zone* zone);

 private:
  // NodeLayout
  Input* InputAt(size_t index) const final;
  bool IsVariadic() const final { return true; }

  ZoneDeque<InputHolder*> inputs_;

  DISALLOW_COPY_AND_ASSIGN(VariadicNodeTemplate);
};

template <typename Base>
VariadicNodeTemplate<Base>::VariadicNodeTemplate(Type* output_type, Zone* zone)
    : Base(output_type), inputs_(zone) {
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
class ELANG_OPTIMIZER_EXPORT PhiOwnerNode
    : public VariadicNodeTemplate<Control> {
  DECLARE_OPTIMIZER_NODE_ABSTRACT_CLASS(PhiOwnerNode, Control);

 public:
  typedef DoubleLinked<PhiNode, PhiOwnerNode> PhiNodes;

  EffectPhiNode* effect_phi() const { return effect_phi_; }
  const PhiNodes& phi_nodes() const { return phi_nodes_; }

 protected:
  PhiOwnerNode(Type* output_type, Zone* zone);

 private:
  void set_effect_phi(EffectPhiNode* effect_phi);

  EffectPhiNode* effect_phi_;
  PhiNodes phi_nodes_;

  DISALLOW_COPY_AND_ASSIGN(PhiOwnerNode);
};

// Concrete classes

//////////////////////////////////////////////////////////////////////
//
// LiteralNodeTemplate
//
#define V(Name, mnemonic, data_type)                                        \
  class ELANG_OPTIMIZER_EXPORT Name##Node final                             \
      : public LiteralNodeTemplate<data_type> {                             \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, LiteralNodeTemplate); \
                                                                            \
   private:                                                                 \
    Name##Node(Type* output_type, data_type data);                          \
                                                                            \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                   \
  };
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Simple nodes
//
#define V(Name, mnemonic, Base)                              \
  class ELANG_OPTIMIZER_EXPORT Name##Node final              \
      : public NodeTemplate<1, Base> {                       \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Base); \
                                                             \
   private:                                                  \
    Name##Node(Type* output_type, Node* input0);             \
                                                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                    \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_1(V)
#undef V

#define V(Name, mnemonic, Base)                                \
  class ELANG_OPTIMIZER_EXPORT Name##Node final                \
      : public NodeTemplate<2, Base> {                         \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Base);   \
                                                               \
   private:                                                    \
    Name##Node(Type* output_type, Node* input0, Node* input1); \
                                                               \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                      \
  };
FOR_EACH_OPTIMIZER_CONCRETE_ARITHMETIC_NODE(V)
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_2(V)
#undef V

#define V(Name, mnemonic, Base)                                              \
  class ELANG_OPTIMIZER_EXPORT Name##Node final                              \
      : public NodeTemplate<3, Base> {                                       \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Base);                 \
                                                                             \
   private:                                                                  \
    Name##Node(Type* output_type, Node* input0, Node* input1, Node* input2); \
                                                                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                                    \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_3(V)
#undef V

#define V(Name, mnemonic, Base)                              \
  class ELANG_OPTIMIZER_EXPORT Name##Node final              \
      : public NodeTemplate<4, Base> {                       \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Base); \
                                                             \
   private:                                                  \
    Name##Node(Type* output_type,                            \
               Node* input0,                                 \
               Node* input1,                                 \
               Node* input2,                                 \
               Node* input3);                                \
                                                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Node);                    \
  };
FOR_EACH_OPTIMIZER_CONCRETE_SIMPLE_NODE_4(V)
#undef V

#define V(Name, mnemonic, Base)                              \
  class ELANG_OPTIMIZER_EXPORT Name##Node final              \
      : public VariadicNodeTemplate<Base> {                  \
    DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(Name##Node, Base); \
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
// EffectPhiNode
//
class ELANG_OPTIMIZER_EXPORT EffectPhiNode final
    : public PhiNodeTemplate<Effect> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(EffectPhiNode, Effect);

 private:
  EffectPhiNode(Type* output_type, Zone* zone, PhiOwnerNode* control);

  DISALLOW_COPY_AND_ASSIGN(EffectPhiNode);
};

//////////////////////////////////////////////////////////////////////
//
// EntryNode
//
class ELANG_OPTIMIZER_EXPORT EntryNode final : public NodeTemplate<0, Control> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(EntryNode, Control);

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
class ELANG_OPTIMIZER_EXPORT FloatCmpNode final : public NodeTemplate<2, Data> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(FloatCmpNode, Data);

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
    : public NodeTemplate<0, Data> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(FunctionReferenceNode, Data);

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
class ELANG_OPTIMIZER_EXPORT GetNode final
    : public ProjectionNodeTemplate<Data> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(GetNode, ProjectionNodeTemplate);

 private:
  GetNode(Type* output_type, Tuple* input, size_t field);

  DISALLOW_COPY_AND_ASSIGN(GetNode);
};

//////////////////////////////////////////////////////////////////////
//
// IntCmpNode
//
class ELANG_OPTIMIZER_EXPORT IntCmpNode final : public NodeTemplate<2, Data> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(IntCmpNode, Data);

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
class ELANG_OPTIMIZER_EXPORT NullNode final : public NodeTemplate<0, Literal> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(NullNode, Literal);

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
    : public NodeTemplate<1, Data> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(ParameterNode, Data);

 public:
  // NodeLayout protocol
  size_t field() const final { return field_; }
  bool has_field() const final { return true; }

 private:
  ParameterNode(Type* output_type, EntryNode* input, size_t field);

  size_t field_;

  DISALLOW_COPY_AND_ASSIGN(ParameterNode);
};

//////////////////////////////////////////////////////////////////////
//
// PhiNode
//
class ELANG_OPTIMIZER_EXPORT PhiNode final
    : public PhiNodeTemplate<Data>,
      public DoubleLinked<PhiNode, PhiOwnerNode>::NodeBase {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(PhiNode, Data);

 public:
  typedef ::elang::optimizer::Node Node;

 private:
  PhiNode(Type* output_type, Zone* zone, PhiOwnerNode* control);

  DISALLOW_COPY_AND_ASSIGN(PhiNode);
};

//////////////////////////////////////////////////////////////////////
//
// ReferenceNode
//
class ELANG_OPTIMIZER_EXPORT ReferenceNode final
    : public NodeTemplate<0, Literal> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(ReferenceNode, Literal);

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
class ELANG_OPTIMIZER_EXPORT SizeOfNode final
    : public NodeTemplate<0, Literal> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(SizeOfNode, Literal);

 public:
  Type* type_operand() const { return type_operand_; }

 private:
  SizeOfNode(Type* uintptr_type, Type* type_operand);

  Type* const type_operand_;

  DISALLOW_COPY_AND_ASSIGN(SizeOfNode);
};

//////////////////////////////////////////////////////////////////////
//
// VoidNode
//
class ELANG_OPTIMIZER_EXPORT VoidNode final : public NodeTemplate<0, Literal> {
  DECLARE_OPTIMIZER_NODE_CONCRETE_CLASS(VoidNode, Literal);

 private:
  explicit VoidNode(Type* output_type);

  DISALLOW_COPY_AND_ASSIGN(VoidNode);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODES_H_
