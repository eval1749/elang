// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTIONS_H_
#define ELANG_HIR_INSTRUCTIONS_H_

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/castable.h"
#include "elang/base/double_linked.h"
#include "elang/base/embedded_container.h"
#include "elang/base/types.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/types_forward.h"

namespace elang {
namespace hir {

// See "instructions_forward.h" for list of all instructions.

// Use-def list node.
class UseDefNode : public DoubleLinked<UseDefNode, Operand>::Node {
 public:
  UseDefNode();
  ~UseDefNode() = default;

  Instruction* instruction() const { return instruction_; }
  UseDefNode* next_user() const { return next(); }
  Operand* operand() const { return operand_; }
  UseDefNode* previous_user() const { return previous(); }

  void Init(Instruction* instruction, Operand* operand);
  void SetOperand(Operand* new_operand);

 private:
  Operand* operand_;

  // Owner of this node which uses |operand_|.
  Instruction* instruction_;

  DISALLOW_COPY_AND_ASSIGN(UseDefNode);
};

#define DECLARE_HIR_OPERAND_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);         \
  friend class Factory;                        \
                                               \
 protected:                                    \
  ~self() override = default;

// Represent an operand in instruction.
class Operand : public Castable, public ZoneAllocated {
  DECLARE_HIR_OPERAND_CLASS(Operand, Castable);

 public:
  typedef DoubleLinked<UseDefNode, Operand> UseDefList;

  Type* type() const { return type_; }
  const UseDefList& users() const { return use_def_list_; }

  virtual void Accept(OperandVisitor* visitor);
  void Use(UseDefNode* holder);
  void Unuse(UseDefNode* holder);

 protected:
  explicit Operand(Type* type);

 private:
  Type* const type_;
  UseDefList use_def_list_;

  DISALLOW_COPY_AND_ASSIGN(Operand);
};

// |Literal| is a base class of literal operand.
class Literal : public Operand {
  DECLARE_HIR_OPERAND_CLASS(Literal, Operand);

 public:
// Literal value getters, e.g. bool_value(), int32_value(), etc.
#define V(Name, name, c_type) virtual c_type name##_value() const;
  FOR_EACH_HIR_LITERAL_OPERAND(V)
#undef V

 protected:
  explicit Literal(Type* type);

 private:
  DISALLOW_COPY_AND_ASSIGN(Literal);
};

class NullLiteral : public Literal {
 private:
  // For initializing per-type singleton null literal.
  friend class PointerType;
  friend class ReferenceType;

  explicit NullLiteral(Type* type);

  DISALLOW_COPY_AND_ASSIGN(NullLiteral);
};

class VoidLiteral : public Literal {
 private:
  // For creating singleton void literal.
  friend class InstructionFactory;
  friend class VoidType;

  VoidLiteral(VoidType* type, int dummy);

  DISALLOW_COPY_AND_ASSIGN(VoidLiteral);
};

#define V(Name, name, cpp_type)                       \
  class Name##Literal : public Literal {              \
    DECLARE_HIR_OPERAND_CLASS(Name##Literal, Literal) \
   public:                                            \
    Name##Literal(Type* type, cpp_type data);         \
                                                      \
   private:                                           \
    cpp_type name##_value() const override;           \
    const cpp_type data_;                             \
    DISALLOW_COPY_AND_ASSIGN(Name##Literal);          \
  };
FOR_EACH_HIR_LITERAL_OPERAND(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Instruction
//
class Instruction : public Operand,
                    public DoubleLinked<Instruction, Block>::Node {
 public:
  Type* output_type() const { return type(); }

  // Returns true if this instruction can be safely removed and doesn't change
  // execution result of containing function. No users for an output of
  // this instruction returns true except for this instruction has no side-
  // effect, e.g. call, store, and so on.
  virtual bool CanBeRemoved() const;

  // Returns number of operands which this instruction has. Other than 'phi',
  // 'pack', and 'switch', number of operands is constant.
  virtual int CountOperands() = 0;

  // Returns true if this transfers control to exit block, e.g. 'jump', 'ret',
  // and 'unreachable'.
  virtual bool IsExit() const;

  // Returns true if this instruction is placed at end of block, e.g. 'br',
  // 'br', 'switch', and so on.
  virtual bool IsTerminator() const;

  // Operand accessor
  virtual Operand* OperandAt(int index) const = 0;
  virtual void SetOperandAt(int index, Operand* new_operand) = 0;

 protected:
  explicit Instruction(Type* output_type);
  ~Instruction() override = default;

 private:
  DoubleLinked<UseDefNode, Instruction> users_;

  DISALLOW_COPY_AND_ASSIGN(Instruction);
};

template <class Derived, typename... Params>
class InstructionTemplate : public Instruction {
 public:
  typedef InstructionTemplate BaseClass;

  static Derived* New(Factory* factory, Type* output_type, Params... params) {
    return new (factory->zone()) Derived(factory, output_type, params...);
  }

  // Instruction
  int CountOperands() override { return sizeof...(Params); }
  Operand* OperandAt(int index) const override {
    return operands_[index].operand();
  }
  void SetOperandAt(int index, Operand* new_operand) override {
    operands_[index].SetOperand(new_operand);
  }

 protected:
  InstructionTemplate(Type* output_type, Params... params)
      : Instruction(output_type) {
    InitOperands(0, params...);
  }

 private:
  void InitOperands(int index) { __assume(index); }
  template <typename... Params>
  void InitOperands(int index, Operand* operand, Params... params) {
    operands_[index].Init(this, operand);
    InitOperands(index + 1, params...);
  }
  EmbeddedContainer<UseDefNode, sizeof...(Params)> operands_;
  DISALLOW_COPY_AND_ASSIGN(InstructionTemplate);
};

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
class CallInstruction
    : public InstructionTemplate<CallInstruction, Operand*, Operand*> {
  DECLARE_HIR_OPERAND_CLASS(CallInstruction, Instruction);

 public:
  // Instruction
  bool CanBeRemoved() const override;

 private:
  friend class BaseClass;

  CallInstruction(Factory* factory,
                  Type* output_type,
                  Operand* callee,
                  Operand* arguments);

  DISALLOW_COPY_AND_ASSIGN(CallInstruction);
};

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
class ReturnInstruction
    : public InstructionTemplate<ReturnInstruction, Operand*> {
  DECLARE_HIR_OPERAND_CLASS(ReturnInstruction, Instruction);

 public:
  static ReturnInstruction* New(Factory* factory, Operand* value);

  // Instruction
  bool IsExit() const override;
  bool IsTerminator() const override;

 private:
  friend class BaseClass;

  ReturnInstruction(Factory* factory, Type* output_type, Operand* value);

  DISALLOW_COPY_AND_ASSIGN(ReturnInstruction);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTIONS_H_
