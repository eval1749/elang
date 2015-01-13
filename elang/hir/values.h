// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_VALUES_H_
#define ELANG_HIR_VALUES_H_

#include <string>
#include <ostream>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/embedded_container.h"
#include "elang/base/float_types.h"
#include "elang/base/visitable.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/values_forward.h"
#include "elang/hir/types_forward.h"

namespace elang {
namespace hir {

class Instruction;

// See "operands_forward.h" for list of all concrete operands.

// Value class hierarchy:
//  Value
//    BasicBlock -- jump target
//    Function -- function literal
//    Instruction -- output of |Instruction| represents SSA register.
//    Literal
//      BoolLiteral
//      CharLiteral
//      Float32Literal Float64Literal
//      Int16Literal Int32Literal Int64Literal Int16Literal
//      NullLiteral -- typed null value for pointer and reference types.
//      StringLiteral
//      UInt16Literal UInt32Literal UInt64Literal UInt16Literal
//      VoidLiteal -- singleton

// Use-def list node.
class ELANG_HIR_EXPORT UseDefNode
    : public DoubleLinked<UseDefNode, Value>::Node {
 public:
  UseDefNode();
  ~UseDefNode() = default;

  Instruction* instruction() const { return instruction_; }
  UseDefNode* next_user() const { return next(); }
  Value* value() const { return value_; }
  UseDefNode* previous_user() const { return previous(); }

  void Init(Instruction* instruction, Value* value);
  void SetValue(Value* new_value);

 private:
  Value* value_;

  // Owner of this node which uses |value_|.
  Instruction* instruction_;

  DISALLOW_COPY_AND_ASSIGN(UseDefNode);
};

#define DECLARE_HIR_VALUE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);       \
  friend class Factory;                      \
                                             \
 protected:                                  \
  ~self() override = default;

// Represent an value in instruction.
class ELANG_HIR_EXPORT Value : public Castable,
                               public Visitable<ValueVisitor>,
                               public ZoneAllocated {
  DECLARE_HIR_VALUE_CLASS(Value, Castable);

 public:
  typedef DoubleLinked<UseDefNode, Value> UseDefList;

  Type* type() const { return type_; }
  const UseDefList& users() const { return use_def_list_; }

  void Use(UseDefNode* holder);
  void Unuse(UseDefNode* holder);

 protected:
  explicit Value(Type* type);

 private:
  Type* const type_;
  UseDefList use_def_list_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

// Print for formatting and debugging.
ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Value& value);

#define DECLARE_HIR_CONCRETE_VALUE_CLASS(self, super) \
  DECLARE_HIR_VALUE_CLASS(self, super);               \
                                                      \
 private:                                             \
  void Accept(ValueVisitor* visitor) final;

// |Literal| is a base class of literal value.
class ELANG_HIR_EXPORT Literal : public Value {
  DECLARE_HIR_VALUE_CLASS(Literal, Value);

 public:
// Literal value getters, e.g. bool_value(), int32_value(), etc.
#define V(Name, name, c_type) virtual c_type name##_value() const;
  FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

 protected:
  explicit Literal(Type* type);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

// Typed null value. |TypeFactory| singleton.
class ELANG_HIR_EXPORT NullLiteral : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(NullLiteral, Literal);

 private:
  // For initializing per-type singleton null literal.
  friend class PointerType;
  friend class ReferenceType;

  explicit NullLiteral(Type* type);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

// Represents reference to object.
class ELANG_HIR_EXPORT Reference : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(Reference, Literal);

 public:
  const base::StringPiece16 name() const { return name_; }

 private:
  Reference(Type* type, base::StringPiece16 name);

  const base::StringPiece16 name_;

  DISALLOW_COPY_AND_ASSIGN(Reference);
};

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Reference& function);

// value for 'void' type. |TypeFactory| singleton.
class ELANG_HIR_EXPORT VoidLiteral : public Literal {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(VoidLiteral, Literal);

 private:
  // For creating singleton void literal.
  friend class InstructionFactory;
  friend class VoidType;

  VoidLiteral(VoidType* type, int dummy);

  DISALLOW_COPY_AND_ASSIGN(VoidLiteral);
};

#define V(Name, name, cpp_type)                              \
  class ELANG_HIR_EXPORT Name##Literal : public Literal {    \
    DECLARE_HIR_CONCRETE_VALUE_CLASS(Name##Literal, Literal) \
   public:                                                   \
    Name##Literal(Type* type, cpp_type data);                \
                                                             \
   private:                                                  \
    cpp_type name##_value() const override;                  \
    const cpp_type data_;                                    \
    DISALLOW_COPY_AND_ASSIGN(Name##Literal);                 \
  };
FOR_EACH_HIR_LITERAL_VALUE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// BasicBlock
//
// You can get predecessors of a basic block from user list in use-def list,
// and successors of a basic block from operands of the last instruction.
// Note: 'ret' and 'unreachable' instructions have 'exit' block as an operand.
//
// This class provides getters only. You need to use |Editor| for
// changing instructions, and |Editor| to add |BasicBlock| to
// |Function|.
//
// TODO(eval1749) Should we add |BasicBlock| to |Function| automatically when
// we insert 'jump' instruction?
//
class ELANG_HIR_EXPORT BasicBlock
    : public Value,
      public DoubleLinked<BasicBlock, Function>::Node {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(BasicBlock, Value);

 public:
  typedef DoubleLinked<Instruction, BasicBlock> InstructionList;

  Function* function() const { return function_; }

  // An integer identifier for debugging.
  int id() const { return id_; }
  void set_id(int id);

  // Getters for instructions in this basic block.
  const InstructionList& instructions() const { return instructions_; }
  Instruction* first_instruction() const;
  Instruction* last_instruction() const;

 private:
  // |Editor| manipulates instruction list
  friend class Editor;

  explicit BasicBlock(Factory* factory);

  // |function_| holds owner of this |BasicBlock|.
  Function* function_;
  // |id_| is assign positive integer by |Editor|. When this basic
  // block is removed from |Function|, |id_| is reset to zero.
  int id_;
  InstructionList instructions_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlock);
};

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const BasicBlock& basic_block);

//////////////////////////////////////////////////////////////////////
//
// Function
//
class ELANG_HIR_EXPORT Function : public Value {
  DECLARE_HIR_CONCRETE_VALUE_CLASS(Function, Value);

 public:
  typedef DoubleLinked<BasicBlock, Function> BasicBlockList;

  const BasicBlockList& basic_blocks() const { return basic_blocks_; }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  FunctionType* function_type() const;

 private:
  friend class Editor;

  Function(Factory* factory, FunctionType* type);

  BasicBlockList basic_blocks_;

  DISALLOW_COPY_AND_ASSIGN(Function);
};

ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Function& function);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_VALUES_H_
