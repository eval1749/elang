// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/lir/testing/lir_test.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/lir/error_data.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {
namespace testing {

LirTest::LirTest() : FactoryUser(new Factory()), factory_(factory()) {
}

std::string LirTest::Commit(Editor* editor) {
  if (editor->Validate(editor->basic_block())) {
    editor->Commit();
    return "";
  }
  std::stringstream ostream;
  ostream << editor->errors();
  return ostream.str();
}

std::vector<Value> LirTest::CollectRegisters(const Function* function) {
  std::vector<Value> registers;
  for (auto const block : function->basic_blocks()) {
    for (auto const phi : block->phi_instructions())
      registers.push_back(phi->output(0));
    for (auto const instr : block->instructions()) {
      for (auto const output : instr->outputs())
        registers.push_back(output);
    }
  }
  return registers;
}

Function* LirTest::CreateFunctionEmptySample() {
  auto const function = factory()->NewFunction();
  Editor editor(factory(), function);
  return function;
}

Function* LirTest::CreateFunctionSample1() {
  auto const function = factory()->NewFunction();
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
  auto const function = CreateFunctionEmptySample();
  auto const exit_block = function->exit_block();
  Editor editor(factory(), function);
  auto const true_block = editor.NewBasicBlock(exit_block);
  auto const false_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  std::vector<Value> values{
      factory()->NewRegister(ValueSize::Size32),
      factory()->NewRegister(ValueSize::Size32),
      factory()->NewRegister(ValueSize::Size32),
  };

  // entry block
  editor.Edit(function->entry_block());
  editor.Append(factory()->NewPCopyInstruction(
      {values[0], values[1]}, {Target::GetParameterAt(values[0], 0),
                               Target::GetParameterAt(values[1], 1)}));
  auto const cond1 = factory()->NewCondition();
  editor.Append(factory()->NewEqInstruction(
      cond1, values[0], factory()->NewIntValue(ValueSize::Size32, 0)));
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
  editor.SetPhiInput(merge_phi, false_block,
                     factory()->NewIntValue(ValueSize::Size32, 42));
  editor.Append(
      factory()->NewCopyInstruction(Target::GetReturn(values[0]), values[2]));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  return function;
}

// The sample block for RemoveCriticalEdges test case:
//   entry:
//    br start
//   start:
//    br sample
//   sample:
//    br %flag1, merge, start
//   merge:
//    phi %1 = start 39, sample 42
//    ret
// An edge sample => merge is a critical edge.
//
Function* LirTest::CreateFunctionWithCriticalEdge() {
  auto const function = CreateFunctionEmptySample();
  auto const entry_block = function->entry_block();
  auto const exit_block = function->exit_block();

  Editor editor(factory(), function);

  Value type(Value::Type::Integer, ValueSize::Size32);
  auto const start_block = editor.NewBasicBlock(exit_block);
  auto const sample_block = editor.NewBasicBlock(exit_block);
  auto const merge_block = editor.NewBasicBlock(exit_block);

  editor.Edit(entry_block);
  editor.SetJump(start_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(start_block);
  editor.SetJump(merge_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(sample_block);
  editor.SetBranch(factory()->NewCondition(), merge_block, start_block);
  EXPECT_EQ("", Commit(&editor));

  editor.Edit(merge_block);
  auto const phi_instr = editor.NewPhi(NewRegister(type));
  editor.SetPhiInput(phi_instr, sample_block, Value::SmallInt32(42));
  editor.SetPhiInput(phi_instr, start_block, Value::SmallInt32(39));
  editor.SetReturn();
  EXPECT_EQ("", Commit(&editor));

  EXPECT_EQ("", Validate(&editor));

  return function;
}

std::vector<Value> LirTest::EmitCopyParameters(Editor* editor,
                                               Value type,
                                               int count) {
  std::vector<Value> registers;
  std::vector<Value> parameters;
  for (auto position = 0; position < count; ++position) {
    registers.push_back(factory()->NewRegister(type));
    parameters.push_back(Target::GetParameterAt(type, position));
  }
  editor->Append(factory()->NewPCopyInstruction(registers, parameters));
  return registers;
}

std::string LirTest::FormatFunction(Editor* editor) {
  std::stringstream ostream;
  if (!editor->Validate()) {
    ostream << editor->errors();
  } else {
    TextFormatter formatter(factory()->literals(), &ostream);
    formatter.FormatFunction(editor->function());
  }
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
  return ostream.str();
}

}  // namespace testing
}  // namespace lir
}  // namespace elang
