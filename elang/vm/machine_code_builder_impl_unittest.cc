// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <memory>
#include <sstream>

#include "elang/vm/factory.h"
#include "elang/vm/machine_code_builder_impl.h"
#include "elang/vm/machine_code_function.h"
#include "gtest/gtest.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilderImplTest offers HIR factories.
//
class MachineCodeBuilderImplTest : public ::testing::Test {
 protected:
  MachineCodeBuilderImplTest();

  MachineCodeBuilderImpl* builder_impl() { return builder_.get(); }
  Factory* factory() { return factory_.get(); }

 private:
  std::unique_ptr<Factory> factory_;
  std::unique_ptr<MachineCodeBuilderImpl> builder_;
};

MachineCodeBuilderImplTest::MachineCodeBuilderImplTest()
    : factory_(new Factory()),
      builder_(new MachineCodeBuilderImpl(factory_.get())) {
}

TEST_F(MachineCodeBuilderImplTest, Basic) {
  auto const builder = static_cast<api::MachineCodeBuilder*>(builder_impl());
#if ELANG_TARGET_ARCH_X64
  std::array<uint8_t, 6> bytes{
      0xB8,  // mov eax, 123
      0x7B,
      0x00,
      0x00,
      0x00,
      0xC3,  // ret
  };
#else
#error "You should provide machine code for MachineCodeBuilderImplTest.Basic"
#endif
  builder->PrepareCode(bytes.size());
  builder->EmitCode(bytes.data(), bytes.size());
  auto const function = builder_impl()->NewMachineCodeFunction();
  EXPECT_EQ(123, function->Call<int>());
}

}  // namespace vm
}  // namespace elang
