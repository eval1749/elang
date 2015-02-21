// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "elang/lir/code_emitter.h"
#include "elang/lir/editor.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/testing/test_machine_code_builder.h"
#include "gtest/gtest.h"

namespace elang {
namespace lir {
namespace {

//////////////////////////////////////////////////////////////////////
//
// CodeEmitterX64Test
//
class CodeEmitterX64Test : public ::testing::Test {
 protected:
  CodeEmitterX64Test() = default;
  ~CodeEmitterX64Test() override = default;

  Factory* factory() { return &factory_; }

 private:
  Factory factory_;

  DISALLOW_COPY_AND_ASSIGN(CodeEmitterX64Test);
};

// Test cases...

TEST_F(CodeEmitterX64Test, Basic) {
  auto const function = factory()->NewFunction({});
  TestMachineCodeBuilder builder;
  CodeEmitter emitter(factory(), &builder);
  emitter.Process(function);
  EXPECT_EQ("0000 C3\n", builder.GetResult());
}

TEST_F(CodeEmitterX64Test, Call) {
  auto const function = factory()->NewFunction({});
  Editor editor(factory(), function);
  auto const entry_block = function->entry_block();
  {
    Editor::ScopedEdit scope(&editor);
    editor.Edit(entry_block);
    auto const callee = factory()->NewStringValue(L"Foo");
    auto const call = factory()->NewCallInstruction(callee);
    editor.InsertBefore(call, entry_block->last_instruction());
  }
  TestMachineCodeBuilder builder;
  CodeEmitter emitter(factory(), &builder);
  emitter.Process(function);
  EXPECT_EQ(
      "string +0001 \"Foo\"\n"
      "0000 E8 00 00 00 00 C3\n",
      builder.GetResult());
}

}  // namespace
}  // namespace lir
}  // namespace elang
