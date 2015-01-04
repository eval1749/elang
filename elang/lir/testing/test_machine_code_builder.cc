// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/testing/test_machine_code_builder.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// TestMachineCodeBuilder
//
TestMachineCodeBuilder::TestMachineCodeBuilder() : code_length_(0) {
}

TestMachineCodeBuilder::~TestMachineCodeBuilder() {
}

std::string TestMachineCodeBuilder::GetResult() {
  auto offset = 0;
  for (auto byte : bytes_) {
    if (!(offset & 15)) {
      if (offset)
        stream_ << std::endl;
      stream_ << base::StringPrintf("%04X", offset);
    }
    stream_ << base::StringPrintf(" %02X", byte);
    ++offset;
  }
  if (offset & 15)
    stream_ << std::endl;
  return stream_.str();
}

// MachineCodeBuilder
void TestMachineCodeBuilder::EmitCode(const uint8_t* codes, int code_length) {
  DCHECK_GE(code_length, 0);
  auto const new_code_length = code_length_ + code_length;
  DCHECK_LE(static_cast<size_t>(new_code_length), bytes_.size());
  ::memcpy(bytes_.data() + code_length_, codes, code_length);
  code_length_ = new_code_length;
}

void TestMachineCodeBuilder::FinishCode() {
  DCHECK_EQ(bytes_.size(), code_length_);
}

void TestMachineCodeBuilder::PrepareCode(int code_length) {
  bytes_.resize(code_length);
}

void TestMachineCodeBuilder::SetCodeOffset(int offset, int target_offset) {
  stream_ << base::StringPrintf("code offset +%04X %d", offset, target_offset)
          << std::endl;
}

void TestMachineCodeBuilder::SetFloat32(int offset, float32_t data) {
  stream_ << base::StringPrintf("float32 +%04X %ff", offset, data) << std::endl;
}

void TestMachineCodeBuilder::SetFloat64(int offset, float64_t data) {
  stream_ << base::StringPrintf("float64 +%04X %f", offset, data) << std::endl;
}

void TestMachineCodeBuilder::SetInt32(int offset, int32_t data) {
  stream_ << base::StringPrintf("int32 +%04X %d", offset, data) << std::endl;
}

void TestMachineCodeBuilder::SetInt64(int offset, int64_t data) {
  stream_ << base::StringPrintf("int64 +%04X %dl", offset, data) << std::endl;
}

void TestMachineCodeBuilder::SetSourceCodeLocation(
    int offset,
    api::SourceCodeLocation location) {
  stream_ << base::StringPrintf("location +%04X %d", location.id) << std::endl;
}

void TestMachineCodeBuilder::SetString(int offset, base::StringPiece16 data) {
  stream_ << base::StringPrintf("string +%04X \"%s\"", offset,
                                base::UTF16ToUTF8(data.as_string()).c_str())
          << std::endl;
}

}  // namespace lir
}  // namespace elang
