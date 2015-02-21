// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <sstream>
#include <string>

#include "elang/lir/testing/lir_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/lir/error_data.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/transforms/prepare_phi_inversion_pass.h"
#include "elang/lir/transforms/register_allocator.h"
#include "elang/lir/transforms/register_assignments.h"
#include "elang/lir/transforms/stack_allocator.h"
#include "elang/lir/transforms/stack_assignments.h"
#include "elang/lir/value.h"

// TODO(eval1749) We should share |std::vector<BasicBlock*> with
// |TextFormatter|.
namespace std {
std::ostream& operator<<(std::ostream& ostream,
                         const std::vector<elang::lir::BasicBlock*>& blocks) {
  ostream << "{";
  auto separator = "";
  for (auto const block : blocks) {
    ostream << separator << *block;
    separator = ", ";
  }
  return ostream << "}";
}
}  // namespace std

namespace elang {
namespace lir {
namespace testing {

namespace {

struct InstructionWithAllocation {
  const RegisterAssignments* assignments;
  Instruction* instruction;

  InstructionWithAllocation(const RegisterAssignments* assignments,
                            Instruction* instruction)
      : assignments(assignments), instruction(instruction) {}
};

// TODO(eval1749) We should share |SortBasicBlocks()| with |TextFormatter|.
std::vector<BasicBlock*> SortBasicBlocks(
    const ZoneUnorderedSet<BasicBlock*>& block_set) {
  std::vector<BasicBlock*> blocks(block_set.begin(), block_set.end());
  std::sort(blocks.begin(), blocks.end(),
            [](BasicBlock* a, BasicBlock* b) { return a->id() < b->id(); });
  return blocks;
}

InstructionWithAllocation PrintWithAllocation(
    const RegisterAssignments& assignments,
    Instruction* instruction) {
  return InstructionWithAllocation(&assignments, instruction);
}

std::ostream& operator<<(std::ostream& ostream,
                         const InstructionWithAllocation& thing) {
  auto const assignments = thing.assignments;
  auto const instr = thing.instruction;
  ostream << instr->opcode();
  if (auto const phi = instr->as<PhiInstruction>()) {
    auto const output = assignments->AllocationOf(phi, phi->output(0));
    ostream << " " << PrintAsGeneric(output) << " = ";
    auto separator = "";
    for (auto const phi_input : phi->phi_inputs()) {
      auto const input = assignments->AllocationOf(phi, phi_input->value());
      ostream << separator << *phi_input->basic_block() << " ";
      ostream << PrintAsGeneric(input);
      separator = ", ";
    }
    return ostream;
  }

  if (!instr->outputs().empty()) {
    auto separator = " ";
    for (auto const output : instr->outputs()) {
      auto const allocation = assignments->AllocationOf(instr, output);
      ostream << separator << PrintAsGeneric(allocation);
      separator = ", ";
    }
    ostream << " =";
  }
  auto separator = " ";
  for (auto const input : instr->inputs()) {
    auto const allocation = assignments->AllocationOf(instr, input);
    ostream << separator << PrintAsGeneric(allocation);
    separator = ", ";
  }
  for (auto const block : instr->block_operands()) {
    ostream << separator << *block;
    separator = ", ";
  }
  return ostream;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// LirTest
//
LirTest::LirTest() : FactoryUser(new Factory()), factory_(factory()) {
}

LirTest::~LirTest() {
}

std::string LirTest::Allocate(Function* function) {
  Editor editor(factory(), function);

  Run<PreparePhiInversionPass>(&editor);

  RegisterAssignments assignments;
  StackAssignments stack_assignments;

  {
    RegisterAllocator allocator(&editor, &assignments, &stack_assignments);
    allocator.Run();
  }

  std::stringstream ostream;
  ostream << *function << ":" << std::endl;
  for (auto const block : function->basic_blocks()) {
    ostream << *block << ":" << std::endl;

    ostream << "  // In: ";
    ostream << SortBasicBlocks(block->predecessors());
    ostream << std::endl;

    ostream << "  // Out: ";
    ostream << SortBasicBlocks(block->successors());
    ostream << std::endl;

    for (auto const phi : block->phi_instructions())
      ostream << "  " << PrintWithAllocation(assignments, phi) << std::endl;
    for (auto const instr : block->instructions()) {
      for (auto action : assignments.BeforeActionOf(instr)) {
        ostream << "* " << PrintWithAllocation(assignments, action)
                << std::endl;
      }
      ostream << "  " << PrintWithAllocation(assignments, instr) << std::endl;
    }
  }
  return ostream.str();
}

std::string LirTest::Commit(Editor* editor) {
  if (editor->Validate(editor->basic_block())) {
    editor->Commit();
    return "";
  }
  std::stringstream ostream;
  ostream << editor->errors();
  ostream << std::endl;
  TextFormatter formatter(factory()->literals(), &ostream);
  formatter.FormatFunction(editor->function());
  return ostream.str();
}

std::vector<Value> LirTest::CollectRegisters(const Function* function) {
  std::vector<Value> registers;
  for (auto const block : function->basic_blocks()) {
    for (auto const phi : block->phi_instructions())
      registers.push_back(phi->output(0));
    for (auto const instr : block->instructions()) {
      for (auto const output : instr->outputs()) {
        if (!output.is_virtual())
          continue;
        registers.push_back(output);
      }
    }
  }
  return registers;
}

Function* LirTest::CreateFunctionEmptySample(
    const std::vector<Value>& parameters) {
  auto const function = factory()->NewFunction(parameters);
  Editor editor(factory(), function);
  return function;
}

Function* LirTest::CreateFunctionSample1() {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  auto const entry_block = function->entry_block();
  {
    Editor::ScopedEdit scope(&editor);
    editor.Edit(entry_block);
    auto const call = factory()->NewCallInstruction(NewStringValue("Foo"));
    editor.InsertBefore(call, entry_block->last_instruction());
  }
  return function;
}

// Populate |Function| with following basic blocks and instructions:
//    function1:
//    block1:
//      // In: {}
//      // Out: {block3, block4}
//      entry
//      pcopy %r1, %r2 = ECX, EDX
//      eq %b2 = %r1, 0
//      br %b2, block3, block4
//    block3: // true block
//      // In: {block1}
//      // Out: {block5}
//      jmp block5
//    block4:  // false block
//      // In: {block1}
//      // Out: {block5}
//      jmp block5
//    block5:
//      // In: {block3, block4}
//      // Out: {block2}
//      phi %r3 = block3 %r2, block4 42
//      mov EAX = %r3
//      ret block2
//    block2:
//      // In: {block5}
//      // Out: {}
//      exit
Function* LirTest::CreateFunctionSample2() {
  std::vector<Value> values{
      factory()->NewRegister(Value::Int32Type()),
      factory()->NewRegister(Value::Int32Type()),
      factory()->NewRegister(Value::Int32Type()),
  };

  std::vector<Value> parameters{
      Target::GetParameterAt(values[0], 0),
      Target::GetParameterAt(values[1], 1),
  };

  auto const function = CreateFunctionEmptySample(parameters);
  auto const exit_block = function->exit_block();
  Editor editor(factory(), function);
  auto const true_block = editor.NewBasicBlock(exit_block);
  auto const false_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  // entry block
  editor.Edit(function->entry_block());
  editor.Append(
      factory()->NewPCopyInstruction({values[0], values[1]}, parameters));
  auto const cond1 = factory()->NewCondition();
  editor.Append(factory()->NewEqInstruction(
      cond1, values[0], Value::SmallInt32(0)));
  editor.SetBranch(cond1, true_block, false_block);
  EXPECT_EQ("", Commit(&editor));

  // true block
  editor.Edit(true_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  // false block
  editor.Edit(false_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  // merge block
  editor.Edit(merge_block);
  auto const merge_phi = editor.NewPhi(values[2]);
  editor.SetPhiInput(merge_phi, true_block, values[1]);
  editor.SetPhiInput(merge_phi, false_block, Value::SmallInt32(42));
  editor.Append(
      factory()->NewCopyInstruction(Target::GetReturn(values[0]), values[2]));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  return function;
}

//  function1:
//  block1:
//    // In: {}
//    // Out: {block2}
//    entry
//    pcopy %r1l, %r2l = RCX, RDX
//    add %r3l = %r1l, %r2l
//    mov RAX = %r3l
//    ret block2
//  block2:
//    // In: {block1}
//    // Out: {}
//    exit
Function* LirTest::CreateFunctionSampleAdd() {
  auto const var0 = NewIntPtrRegister();
  auto const var1 = NewIntPtrRegister();
  auto const var2 = NewIntPtrRegister();
  std::vector<Value> parameters{
      Target::GetParameterAt(var0, 0), Target::GetParameterAt(var1, 1),
  };
  auto const function = CreateFunctionEmptySample(parameters);
  Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewPCopyInstruction({var0, var1}, parameters));
  editor.Append(factory()->NewAddInstruction(var2, var0, var1));
  editor.Append(factory()->NewCopyInstruction(Target::GetReturn(var2), var2));
  EXPECT_EQ("", Commit(&editor));
  return function;
}

// The sample block for RemoveCriticalEdges test case:
//   entry:
//    br start
//   start:
//    br %flag1, sample2, sample
//   sample:
//    br %flag2, merge, start
//   sample2:
//    jump merge
//   merge:
//    phi %1 = start 39, sample 42
//    mov EAX = %1
//    ret
// An edge sample => merge is a critical edge.
//
Function* LirTest::CreateFunctionWithCriticalEdge() {
  auto const function = CreateFunctionEmptySample({});
  auto const entry_block = function->entry_block();
  auto const exit_block = function->exit_block();

  Editor editor(factory(), function);

  auto const type = Value::Int32Type();
  auto const start_block = editor.NewBasicBlock(exit_block);
  auto const sample_block = editor.NewBasicBlock(exit_block);
  auto const sample2_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  editor.Edit(entry_block);
  editor.SetJump(start_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(start_block);
  editor.SetBranch(factory()->NewCondition(), sample2_block, sample_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(sample_block);
  editor.SetBranch(factory()->NewCondition(), merge_block, start_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(sample2_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(merge_block);
  auto const phi_instr = editor.NewPhi(NewRegister(type));
  editor.SetPhiInput(phi_instr, sample_block, Value::SmallInt32(42));
  editor.SetPhiInput(phi_instr, sample2_block, Value::SmallInt32(39));
  editor.Append(factory()->NewCopyInstruction(
      Target::GetReturn(phi_instr->output(0)), phi_instr->output(0)));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  EXPECT_EQ("", Validate(&editor));

  return function;
}

std::string LirTest::FormatFunction(Editor* editor) {
  auto const result = Validate(editor);
  if (result != "")
    return result;
  std::stringstream ostream;
  TextFormatter formatter(factory()->literals(), &ostream);
  formatter.FormatFunction(editor->function());
  return ostream.str();
}

Literal* LirTest::GetLiteral(Value value) {
  return factory()->GetLiteral(value);
}

Value LirTest::NewFloat32Value(float32_t data) {
  return factory()->NewFloat32Value(data);
}

Value LirTest::NewFloat64Value(float64_t data) {
  return factory()->NewFloat64Value(data);
}

Value LirTest::NewIntValue(ValueSize size, int64_t data) {
  return factory()->NewIntValue(size, data);
}

Value LirTest::NewIntPtrRegister() {
  return NewRegister(Target::IntPtrType());
}

Value LirTest::NewStringValue(base::StringPiece16 data) {
  return factory()->NewStringValue(data);
}
Value LirTest::NewStringValue(base::StringPiece data) {
  return factory()->NewStringValue(base::UTF8ToUTF16(data));
}

std::string LirTest::Validate(Editor* editor) {
  if (editor->Validate())
    return "";
  std::stringstream ostream;
  ostream << editor->errors();
  ostream << std::endl;
  TextFormatter formatter(factory()->literals(), &ostream);
  formatter.FormatFunction(editor->function());
  return ostream.str();
}

}  // namespace testing
}  // namespace lir
}  // namespace elang
