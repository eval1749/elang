// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/editor.h"
#include "elang/hir/error_code.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/testing/hir_test.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirInstructionTest offers HIR factories.
//
class HirInstructionTest : public testing::HirTest {
 protected:
  HirInstructionTest() = default;
  ~HirInstructionTest() override = default;

  Instruction* NewConsumer(Type* input_type);
  Instruction* NewSource(Type* output_type);

 private:
  DISALLOW_COPY_AND_ASSIGN(HirInstructionTest);
};

Instruction* HirInstructionTest::NewConsumer(Type* input_type) {
  auto const name = factory()->NewAtomicString(L"Consumer");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type(), input_type), name);
  return factory()->NewCallInstruction(callee, input_type->default_value());
}

Instruction* HirInstructionTest::NewSource(Type* output_type) {
  auto const name = factory()->NewAtomicString(L"Source");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(output_type, void_type()), name);
  return factory()->NewCallInstruction(callee, void_value());
}

// Arithmetic binary operations
// Bitwise binary operations
// Bitwise shift operations
#define V(Name, mnemonic, ...)                                              \
  TEST_F(HirInstructionTest, Name##Instruction) {                           \
    auto const left = NewSource(int32_type());                              \
    auto const right = editor()->NewInt32(1234);                            \
    auto const instr = editor()->factory()->New##Name##Instruction(         \
        int32_type(), left, right);                                         \
    editor()->Edit(entry_block());                                          \
    editor()->Append(left);                                                 \
    editor()->Append(instr);                                                \
    editor()->Commit();                                                     \
    EXPECT_EQ("bb1:5:int32 %r5 = " mnemonic " %r4, 1234", ToString(instr)); \
    editor()->Edit(entry_block());                                          \
    editor()->SetInput(instr, 0, factory()->NewFloat32Literal(1.234f));     \
    editor()->Commit();                                                     \
    EXPECT_EQ("Validate.Instruction.Type bb1:5:int32 %r5 = " mnemonic       \
              " 1.234f, 1234 0\n",                                          \
              Validate());                                                  \
  }
FOR_EACH_ARITHMETIC_BINARY_OPERATION(V)
FOR_EACH_BITWISE_BINARY_OPERATION(V)
FOR_EACH_BITWISE_SHIFT_OPERATION(V)
#undef V

// Equality and Relational
#define V(Name, mnemonic, ...)                                             \
  TEST_F(HirInstructionTest, Name##Instruction) {                          \
    auto const left = NewSource(int32_type());                             \
    auto const right = editor()->NewInt32(1234);                           \
    auto const instr =                                                     \
        editor()->factory()->New##Name##Instruction(left, right);          \
    editor()->Edit(entry_block());                                         \
    editor()->Append(left);                                                \
    editor()->Append(instr);                                               \
    editor()->Commit();                                                    \
    EXPECT_EQ("bb1:5:bool %b5 = " mnemonic " %r4, 1234", ToString(instr)); \
    editor()->Edit(entry_block());                                         \
    editor()->SetInput(instr, 0, factory()->NewFloat32Literal(1.234f));    \
    editor()->Commit();                                                    \
    EXPECT_EQ("Validate.Instruction.Type bb1:5:bool %b5 = " mnemonic       \
              " 1.234f, 1234 1\n",                                         \
              Validate());                                                 \
  }
FOR_EACH_EQUALITY_OPERATION(V)
FOR_EACH_RELATIONAL_OPERATION(V)
#undef V

// Type cast operation
#define V(Name, mnemonic, ...)                                              \
  TEST_F(HirInstructionTest, Name##Instruction) {                           \
    auto const input = editor()->NewInt32(1234);                            \
    auto const instr =                                                      \
        editor()->factory()->New##Name##Instruction(float64_type(), input); \
    editor()->Edit(entry_block());                                          \
    editor()->Append(instr);                                                \
    editor()->Commit();                                                     \
    EXPECT_EQ("", Validate());                                              \
    EXPECT_EQ("bb1:4:float64 %f4 = " mnemonic " 1234", ToString(instr));    \
  }
FOR_EACH_TYPE_CAST_OPERATION(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// BoundInstruction
//
TEST_F(HirInstructionTest, BoundInstruction) {
  editor()->Edit(entry_block());
  auto const array_type = types()->NewArrayType(float64_type(), {1});
  auto const array_pointer = NewSource(types()->NewPointerType(array_type));
  editor()->Append(array_pointer);
  auto const bound =
      factory()->NewBound(array_pointer, factory()->NewInt32Literal(42));
  editor()->Append(bound);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_EQ("bb1:5:bool %b5 = bound %p4, 42", ToString(bound));
}

//////////////////////////////////////////////////////////////////////
//
// BranchInstruction
//
// Conditional
TEST_F(HirInstructionTest, BranchInstruction) {
  auto const true_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  auto const false_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  editor()->Edit(entry_block());
  auto const call_instr = NewSource(bool_type());
  editor()->Append(call_instr);
  editor()->SetBranch(call_instr, true_block, false_block);
  editor()->Commit();
  EXPECT_EQ("", Validate());

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(3, instr->CountInputs());
  EXPECT_EQ(call_instr, instr->input(0));
  EXPECT_EQ(true_block, instr->input(1));
  EXPECT_EQ(false_block, instr->input(2));
  EXPECT_EQ("bb1:7:br %b6, block3, block4", ToString(instr));
}

TEST_F(HirInstructionTest, BranchUncoditional) {
  auto const target_block = editor()->EditNewBasicBlock();
  editor()->SetReturn(void_value());
  editor()->Commit();

  editor()->Edit(entry_block());
  editor()->SetBranch(target_block);
  editor()->Commit();

  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountInputs());
  EXPECT_EQ(target_block, instr->input(0));
  EXPECT_EQ("", Validate());
  EXPECT_EQ("bb1:5:br block3", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// CallInstruction
//
TEST_F(HirInstructionTest, CallInstruction) {
  auto const callee_name = factory()->NewAtomicString(L"Console.WriteLine");
  auto const callee = factory()->NewReference(
      types()->NewFunctionType(void_type(), string_type()), callee_name);
  auto const args = factory()->NewStringLiteral(L"foo");
  auto const instr = factory()->NewCallInstruction(callee, args);
  editor()->Edit(entry_block());
  editor()->Append(instr);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(callee, instr->input(0));
  EXPECT_EQ(args, instr->input(1));
  EXPECT_EQ("bb1:4:call `Console.WriteLine`, \"foo\"", ToString(instr));

  auto callee_found = false;
  for (auto const user : callee->users()) {
    if (user->instruction() == instr) {
      callee_found = true;
      break;
    }
  }
  EXPECT_TRUE(callee_found) << "call instruction must be a user of callee.";
}

//////////////////////////////////////////////////////////////////////
//
// ElementInstruction
//
TEST_F(HirInstructionTest, ElementInstruction) {
  editor()->Edit(entry_block());
  auto const array_type = types()->NewArrayType(float64_type(), {1});
  auto const array_pointer = NewSource(types()->NewPointerType(array_type));
  editor()->Append(array_pointer);
  auto const element =
      factory()->NewElement(array_pointer, factory()->NewInt32Literal(42));
  editor()->Append(element);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_EQ("bb1:5:float64* %p5 = element %p4, 42", ToString(element));
}

//////////////////////////////////////////////////////////////////////
//
// GetInstruction
//
TEST_F(HirInstructionTest, GetInstruction) {
  auto const parameters_type =
      types()->NewTupleType({int32_type(), bool_type()});
  auto const function = NewFunction(void_type(), parameters_type);
  auto const entry = function->entry_block()->first_instruction();
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  auto const get0 =
      factory()->NewGetInstruction(entry, 0)->as<GetInstruction>();
  EXPECT_EQ(0, get0->index());
  auto const get1 =
      factory()->NewGetInstruction(entry, 1)->as<GetInstruction>();
  auto const get2 =
      factory()->NewGetInstruction(entry, 1)->as<GetInstruction>();
  EXPECT_EQ(1, get1->index());
  editor.Append(get0);
  editor.Append(get1);
  editor.Append(
      factory()->NewNeInstruction(get0, int32_type()->default_value()));
  // |get2| doesn't confirm 'get' instruciton ordering restriction.
  editor.Append(get2);
  editor.Commit();
  EXPECT_FALSE(editor.Validate());
  EXPECT_EQ("bb3:7:int32 %r7 = get %t5, 0", ToString(get0));
  EXPECT_EQ("bb3:8:bool %b8 = get %t5, 1", ToString(get1));
  EXPECT_EQ("Validate.Instruction.Get bb3:10:bool %b10 = get %t5, 1\n",
            GetErrors(editor));
}

//////////////////////////////////////////////////////////////////////
//
// IfInstruction
//
TEST_F(HirInstructionTest, IfInstruction) {
  auto const true_value = editor()->NewInt32(12);
  auto const false_value = editor()->NewInt32(34);
  auto const condition = NewSource(bool_type());
  auto const instr = factory()->NewIfInstruction(int32_type(), condition,
                                                 true_value, false_value);
  editor()->Edit(entry_block());
  editor()->Append(condition);
  editor()->Append(instr);
  editor()->Commit();
  EXPECT_EQ("bb1:5:int32 %r5 = if %b4, 12, 34", ToString(instr));
  editor()->Edit(entry_block());
  editor()->SetInput(instr, 1, factory()->NewFloat32Literal(3.4f));
  editor()->Commit();
  EXPECT_EQ("Validate.Instruction.Type bb1:5:int32 %r5 = if %b4, 3.4f, 34 1\n",
            Validate());
}

//////////////////////////////////////////////////////////////////////
//
// LoadInstruction
//
TEST_F(HirInstructionTest, LoadInstruction) {
  auto const bool_pointer_type = types()->NewPointerType(bool_type());
  auto const source = NewSource(bool_pointer_type);
  auto const instr = factory()->NewLoadInstruction(source);
  editor()->Edit(entry_block());
  editor()->Append(source);
  editor()->Append(instr);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_TRUE(instr->MaybeUseless());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(bool_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountInputs());
  EXPECT_EQ(source, instr->input(0));
  EXPECT_EQ("bb1:5:bool %b5 = load %p4", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// PhiInstruction
//
TEST_F(HirInstructionTest, PhiInstruction) {
  // Create diamond graph
  auto const merge_block =
      editor()->SplitBefore(entry_block()->last_instruction());

  auto const true_block = editor()->EditNewBasicBlock(merge_block);
  editor()->SetBranch(merge_block);
  editor()->Commit();

  auto const false_block = editor()->EditNewBasicBlock(merge_block);
  editor()->SetBranch(merge_block);
  editor()->Commit();

  editor()->Continue(entry_block());
  auto const call_instr = NewSource(bool_type());
  editor()->Append(call_instr);
  editor()->SetBranch(call_instr, true_block, false_block);
  editor()->Commit();

  editor()->Edit(merge_block);
  auto const phi = editor()->NewPhi(bool_type());
  editor()->SetPhiInput(phi, true_block, true_value());
  editor()->SetPhiInput(phi, false_block, false_value());
  auto const consumer = NewConsumer(bool_type());
  editor()->Append(consumer);
  editor()->SetInput(consumer, 1, phi);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_EQ("bb3:8:bool %b8 = phi block4 true, block5 false", ToString(phi));
}

//////////////////////////////////////////////////////////////////////
//
// ReturnInstruction
//
TEST_F(HirInstructionTest, ReturnInstruction) {
  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(void_value(), instr->input(0));
  EXPECT_EQ(exit_block(), instr->input(1));
  EXPECT_EQ("bb1:3:ret void, block2", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// StoreInstruction
//
TEST_F(HirInstructionTest, StoreInstruction) {
  auto const bool_pointer_type = types()->NewPointerType(bool_type());
  auto const source = NewSource(bool_pointer_type);
  auto const value = bool_type()->default_value();
  auto const instr = factory()->NewStoreInstruction(source, value);
  editor()->Edit(entry_block());
  editor()->Append(source);
  editor()->Append(instr);
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(source, instr->input(0));
  EXPECT_EQ(value, instr->input(1));
  EXPECT_EQ("bb1:5:store %p4, false", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// StackAllocInstruction
//
TEST_F(HirInstructionTest, StackAllocInstruction) {
  editor()->Edit(entry_block());
  auto const instr = factory()->NewStackAlloc(int32_type(), 3);
  editor()->Append(instr);
  editor()->Append(factory()->NewLoadInstruction(instr));
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(types()->NewPointerType(int32_type()), instr->output_type());
  EXPECT_EQ(0, instr->CountInputs());
  EXPECT_EQ("bb1:4:int32* %p4 = alloca 3", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// ThrowInstruction
//
TEST_F(HirInstructionTest, ThrowInstruction) {
  editor()->Edit(entry_block());
  editor()->SetThrow(false_value());
  editor()->Commit();
  EXPECT_EQ("", Validate());
  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ(false_value(), instr->input(0));
  EXPECT_EQ(exit_block(), instr->input(1));
  EXPECT_EQ("bb1:4:throw false, block2", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// TupleInstruction
//
TEST_F(HirInstructionTest, TupleInstruction) {
  editor()->Edit(entry_block());
  auto const type = types()->NewTupleType({int32_type(), bool_type()});
  auto const instr = factory()->NewTuple(
      type, {int32_type()->default_value(), bool_type()->default_value()});
  editor()->Append(instr);
  editor()->Append(factory()->NewGetInstruction(instr, 0));
  editor()->Commit();
  EXPECT_EQ("", Validate());
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_FALSE(instr->IsTerminator());
  EXPECT_EQ(type, instr->output_type());
  EXPECT_EQ(2, instr->CountInputs());
  EXPECT_EQ("bb1:4:{int32, bool} %t4 = tuple 0, false", ToString(instr));
}

//////////////////////////////////////////////////////////////////////
//
// UnreachableInstruction
//
TEST_F(HirInstructionTest, UnreachableInstruction) {
  editor()->Edit(entry_block());
  editor()->SetUnreachable();
  editor()->Commit();
  EXPECT_EQ("", Validate());
  auto const instr = entry_block()->last_instruction();
  EXPECT_FALSE(instr->MaybeUseless());
  EXPECT_TRUE(instr->IsTerminator());
  EXPECT_EQ(void_type(), instr->output_type());
  EXPECT_EQ(1, instr->CountInputs());
  EXPECT_EQ(exit_block(), instr->input(0));
  EXPECT_EQ("bb1:4:unreachable block2", ToString(instr));
}

}  // namespace hir
}  // namespace elang
