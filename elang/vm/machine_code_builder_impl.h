// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_MACHINE_CODE_BUILDER_IMPL_H_
#define ELANG_VM_MACHINE_CODE_BUILDER_IMPL_H_

#include <memory>
#include <string>

#include "elang/api/machine_code_builder.h"

namespace elang {
namespace targets {
class CodeBuffer;
}
namespace vm {

class Factory;
class MachineCodeFunction;

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilderImpl
//
class MachineCodeBuilderImpl final : public api::MachineCodeBuilder {
 public:
  explicit MachineCodeBuilderImpl(Factory* factory);
  ~MachineCodeBuilderImpl() final;

  MachineCodeFunction* NewMachineCodeFunction();

 private:
  class CodeBuffer;

  // api::MachineCodeBuilder
  void EmitCode(const uint8_t* bytes, size_t code_size) final;
  void FinishCode() final;
  void PrepareCode(size_t code_size) final;
  void SetCallSite(size_t offset, base::StringPiece16 string) final;
  void SetCodeOffset(size_t offset, size_t target_offset) final;
  void SetFloat32(size_t offset, float32_t float32) final;
  void SetFloat64(size_t offset, float64_t float64) final;
  void SetInt32(size_t offset, int32_t int32) final;
  void SetInt64(size_t offset, int64_t int64) final;
  void SetSourceCodeLocation(size_t offset,
                             api::SourceCodeLocation location) final;
  void SetString(size_t offset, base::StringPiece16 string) final;

  std::unique_ptr<CodeBuffer> code_buffer_;
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(MachineCodeBuilderImpl);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_MACHINE_CODE_BUILDER_IMPL_H_
