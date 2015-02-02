// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/cg/testing/cg_test.h"

#include "elang/cg/generator.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/instructions.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"

namespace elang {
namespace cg {
namespace {

//////////////////////////////////////////////////////////////////////
//
// GeneratorX64Test
//
class GeneratorX64Test : public testing::CgTest {
 protected:
  GeneratorX64Test() = default;
  ~GeneratorX64Test() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(GeneratorX64Test);
};

// Test cases...

TEST_F(GeneratorX64Test, Basic) {
  Generator generator(lir_factory(), function());
  auto const result = generator.Generate();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Format(result));
}

TEST_F(GeneratorX64Test, Parameter) {
  auto const function =
      NewFunction(void_type(), types()->NewTupleType({int32_type(),
                                                      int64_type(),
                                                      bool_type(),
                                                      float64_type(),
                                                      int64_type()}));
  auto const entry = function->entry_block()->first_instruction();
  auto const num_parameters = entry->type()->as<hir::TupleType>()->size();
  hir::Editor editor(factory(), function);
  editor.Edit(function->entry_block());
  for (auto position = 0; position < num_parameters; ++position)
    editor.Append(factory()->NewGetInstruction(entry, position));
  editor.Commit();
  ASSERT_TRUE(editor.Validate());

  Generator generator(lir_factory(), function);
  auto const result = generator.Generate();
  EXPECT_EQ(
      "function1:\n"
      "block1:\n"
      "  entry\n"
      "  mov %r1, ECX\n"
      "  mov %r2, RDX\n"
      "  mov %r3, R8D\n"
      "  mov %f1, XMM3\n"
      "  mov %r4, %param[4]\n"
      "  ret\n"
      "block2:\n"
      "  exit\n",
      Format(result));
}

}  // namespace
}  // namespace cg
}  // namespace elang
