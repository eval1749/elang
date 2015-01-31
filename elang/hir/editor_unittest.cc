// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <sstream>

#include "elang/base/zone.h"
#include "elang/hir/editor.h"
#include "elang/hir/factory.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/instructions.h"
#include "elang/hir/values.h"
#include "elang/hir/types.h"
#include "elang/hir/testing/hir_test.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// HirEditorTest offers HIR factories.
//
class HirEditorTest : public testing::HirTest {
 protected:
  HirEditorTest() = default;
  ~HirEditorTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(HirEditorTest);
};

// Functions
TEST_F(HirEditorTest, ValidateError) {
  editor()->EditNewBasicBlock();
  EXPECT_FALSE(editor()->Commit());

  editor()->Edit(entry_block());
  editor()->SetInput(entry_block()->last_instruction(), 0, false_value());
  EXPECT_FALSE(editor()->Commit());
  EXPECT_EQ(
      "Validate.BasicBlock.Empty block3\n"
      "Validate.Instruction.Operand bb1:3:ret false, block2 0 void\n",
      GetErrors());
}

}  // namespace hir
}  // namespace elang
