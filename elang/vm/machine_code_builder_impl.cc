// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/vm/machine_code_builder_impl.h"

#include "base/logging.h"
#include "elang/vm/factory.h"
#include "elang/vm/machine_code_function.h"

namespace elang {
namespace vm {

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
  auto const blob = factory_->NewCodeBlob(code_size());
  ::memcpy(blob, bytes_.data(), code_size());
  return new (factory_) MachineCodeFunction(blob, code_size());
}

// api::MachineCodeBuilder
void MachineCodeBuilderImpl::EmitCode(const uint8_t* codes, int code_size) {
  auto const offset = bytes_.size();
  bytes_.resize(offset + code_size);
  ::memcpy(bytes_.data() + offset, codes, code_size);
}

void MachineCodeBuilderImpl::FinishCode() {
}

void MachineCodeBuilderImpl::PrepareCode(int code_size) {
  bytes_.reserve(code_size);
}

void MachineCodeBuilderImpl::SetCodeOffset(int offset, int target_offset) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  DCHECK_LT(static_cast<size_t>(target_offset), code_size());
}

void MachineCodeBuilderImpl::SetFloat32(int offset, float32_t data) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(data);
}

void MachineCodeBuilderImpl::SetFloat64(int offset, float64_t data) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(data);
}

void MachineCodeBuilderImpl::SetInt32(int offset, int32_t data) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(data);
}

void MachineCodeBuilderImpl::SetInt64(int offset, int64_t data) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(data);
}

void MachineCodeBuilderImpl::SetSourceCodeLocation(
    int offset,
    api::SourceCodeLocation location) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(location);
}

void MachineCodeBuilderImpl::SetString(int offset, base::StringPiece16 data) {
  DCHECK_LT(static_cast<size_t>(offset), code_size());
  __noop(data);
}

}  // namespace vm
}  // namespace elang
