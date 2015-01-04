// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_TESTING_TEST_MACHINE_CODE_BUILDER_H_
#define ELANG_LIR_TESTING_TEST_MACHINE_CODE_BUILDER_H_

#include <sstream>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "elang/api/machine_code_builder.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilder
//
class TestMachineCodeBuilder final : public api::MachineCodeBuilder {
 public:
  TestMachineCodeBuilder();
  ~TestMachineCodeBuilder() final;

  std::string GetResult();

 private:
  void EmitCode(const uint8_t* codes, int code_length) final;
  void FinishCode() final;
  void PrepareCode(int code_length) final;
  void SetCodeOffset(int offset, int target_offset) final;
  void SetFloat32(int offset, float32_t data) final;
  void SetFloat64(int offset, float64_t data) final;
  void SetInt32(int offset, int32_t data) final;
  void SetInt64(int offset, int64_t data) final;
  void SetSourceCodeLocation(int offset,
                             api::SourceCodeLocation location) final;
  void SetString(int offset, base::StringPiece16 data) final;

 private:
  std::vector<uint8_t> bytes_;
  int code_length_;
  std::stringstream stream_;

  DISALLOW_COPY_AND_ASSIGN(TestMachineCodeBuilder);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TESTING_TEST_MACHINE_CODE_BUILDER_H_
