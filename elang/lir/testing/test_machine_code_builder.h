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
  void EmitCode(const uint8_t* bytes, size_t size) final;
  void FinishCode() final;
  void PrepareCode(size_t size) final;
  void SetCallSite(size_t offset, base::StringPiece16 string) final;
  void SetCodeOffset(size_t offset, size_t target_offset) final;
  void SetFloat32(size_t offset, float32_t data) final;
  void SetFloat64(size_t offset, float64_t data) final;
  void SetInt32(size_t offset, int32_t data) final;
  void SetInt64(size_t offset, int64_t data) final;
  void SetSourceCodeLocation(size_t offset,
                             api::SourceCodeLocation location) final;
  void SetString(size_t offset, base::StringPiece16 data) final;

 private:
  std::vector<uint8_t> bytes_;
  size_t size_;
  std::stringstream stream_;

  DISALLOW_COPY_AND_ASSIGN(TestMachineCodeBuilder);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_TESTING_TEST_MACHINE_CODE_BUILDER_H_
