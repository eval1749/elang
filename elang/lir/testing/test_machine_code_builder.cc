// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <vector>

#include "elang/lir/testing/test_machine_code_builder.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"

namespace elang {
namespace lir {

namespace {
//////////////////////////////////////////////////////////////////////
//
// BytesPrinter
//
class BytesPrinter final {
 public:
  explicit BytesPrinter(std::ostream* ostream)
      : offset_(0), ostream_(*ostream), repeated_(0x100), repeat_times_(0) {}

  ~BytesPrinter() {
    PrintRepat();
    PrintBytes();
  }

  void Feed(uint8_t byte) {
    bytes_.push_back(byte);
    if (bytes_.size() < 16)
      return;

    if (IsRepeated()) {
      bytes_.resize(0);
      repeat_times_ += 16;
      return;
    }

    PrintRepat();

    repeated_ = byte;
    if (IsRepeated()) {
      bytes_.resize(0);
      repeat_times_ = 16;
      return;
    }

    PrintBytes();
  }

 private:
  bool IsRepeated() const {
    for (auto const byte : bytes_) {
      if (byte != repeated_)
        return false;
    }
    return true;
  }

  void PrintRepat() {
    if (!repeat_times_)
      return;
    ostream_ << base::StringPrintf("%04X ... 0x%02X x %d ...", offset_,
                                   repeated_, repeat_times_);
    ostream_ << std::endl;
    offset_ += repeat_times_;
    repeated_ = 0x100;
    repeat_times_ = 0;
  }

  void PrintBytes() {
    if (bytes_.empty())
      return;
    ostream_ << base::StringPrintf("%04X", offset_);
    for (auto byte : bytes_)
      ostream_ << base::StringPrintf(" %02X", byte);
    ostream_ << std::endl;
    offset_ += static_cast<int>(bytes_.size());
    bytes_.resize(0);
  }

  std::vector<uint8_t> bytes_;
  int offset_;
  int repeated_;
  int repeat_times_;
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(BytesPrinter);
};

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TestMachineCodeBuilder
//
TestMachineCodeBuilder::TestMachineCodeBuilder() : code_length_(0) {
}

TestMachineCodeBuilder::~TestMachineCodeBuilder() {
}

std::string TestMachineCodeBuilder::GetResult() {
  {
    BytesPrinter printer(&stream_);
    for (auto const byte : bytes_)
      printer.Feed(byte);
  }
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

void TestMachineCodeBuilder::SetCallSite(int offset,
                                         base::StringPiece16 callee) {
  stream_ << base::StringPrintf("call site +%04X ", offset)
          << callee.as_string() << std::endl;
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
