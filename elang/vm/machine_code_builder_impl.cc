// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/vm/machine_code_builder_impl.h"

#include "base/logging.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/targets/bytes.h"
#include "elang/vm/factory.h"
#include "elang/vm/machine_code_collection.h"
#include "elang/vm/machine_code_function.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilderImpl::CodeBuffer
//
class MachineCodeBuilderImpl::CodeBuffer final {
 public:
  CodeBuffer(uint8_t* bytes, size_t size);
  ~CodeBuffer() = default;

  EntryPoint entry_point() const {
    return reinterpret_cast<EntryPoint>(const_cast<uint8_t*>(bytes_.bytes()));
  }
  size_t size() const { return bytes_.size(); }

  void Append(const uint8_t* bytes, size_t size);
  void SetRelativeAddress32(int offset, const uint8_t* address);

 private:
  targets::Bytes bytes_;
  size_t size_;

  DISALLOW_COPY_AND_ASSIGN(CodeBuffer);
};

MachineCodeBuilderImpl::CodeBuffer::CodeBuffer(uint8_t* bytes, size_t size)
    : bytes_(bytes, size), size_(0) {
}

void MachineCodeBuilderImpl::CodeBuffer::Append(const uint8_t* bytes,
                                                size_t size) {
  auto const new_size = size_ + size;
  DCHECK_LE(new_size, bytes_.size());
  bytes_.SetBytes(size_, bytes, size);
  size_ = new_size;
}

void MachineCodeBuilderImpl::CodeBuffer::SetRelativeAddress32(
    int offset,
    const uint8_t* address) {
  DCHECK_LE(offset + 4, size_);
  bytes_.SetRelativeAddress32(offset, address);
}

//////////////////////////////////////////////////////////////////////
//
// MachineCodeBuilderImpl
//
MachineCodeBuilderImpl::MachineCodeBuilderImpl(Factory* factory)
    : factory_(factory) {
}

MachineCodeBuilderImpl::~MachineCodeBuilderImpl() {
}

MachineCodeFunction* MachineCodeBuilderImpl::NewMachineCodeFunction() {
  return new (factory_) MachineCodeFunction(
      code_buffer_->entry_point(), static_cast<int>(code_buffer_->size()), {});
}

// api::MachineCodeBuilder
void MachineCodeBuilderImpl::EmitCode(const uint8_t* bytes, int size) {
  DCHECK(code_buffer_) << "You should call Prepare(code_size).";
  code_buffer_->Append(bytes, size);
}

void MachineCodeBuilderImpl::FinishCode() {
}

void MachineCodeBuilderImpl::PrepareCode(int code_size) {
  DCHECK(!code_buffer_);
  auto const bytes =
      reinterpret_cast<uint8_t*>(factory_->NewCodeBlob(code_size));
  code_buffer_.reset(new CodeBuffer(bytes, code_size));
}

void MachineCodeBuilderImpl::SetCallSite(int offset,
                                         base::StringPiece16 string) {
  auto const name = factory_->NewAtomicString(string);
  auto const function =
      factory_->machine_code_collection()->FunctionByName(name);
  DCHECK(function);
  code_buffer_->SetRelativeAddress32(offset,
                                     function->code_start_for_testing());
}

void MachineCodeBuilderImpl::SetCodeOffset(int offset, int target_offset) {
}

void MachineCodeBuilderImpl::SetFloat32(int offset, float32_t data) {
}

void MachineCodeBuilderImpl::SetFloat64(int offset, float64_t data) {
}

void MachineCodeBuilderImpl::SetInt32(int offset, int32_t data) {
}

void MachineCodeBuilderImpl::SetInt64(int offset, int64_t data) {
}

void MachineCodeBuilderImpl::SetSourceCodeLocation(
    int offset,
    api::SourceCodeLocation location) {
}

void MachineCodeBuilderImpl::SetString(int offset, base::StringPiece16 data) {
}

}  // namespace vm
}  // namespace elang
