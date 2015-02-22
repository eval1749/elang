// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/code_buffer.h"

#include "base/logging.h"
#include "elang/api/machine_code_builder.h"
#include "elang/base/zone_allocated.h"
#include "elang/lir/emitters/value_emitter.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::BasicBlockData
//
struct CodeBuffer::BasicBlockData : ZoneAllocated {
  int buffer_offset;
  int code_length;
  int code_offset;

  explicit BasicBlockData(int buffer_offset)
      : buffer_offset(buffer_offset),
        code_length(0),
        code_offset(buffer_offset) {}
};

static_assert(sizeof(CodeBuffer::BasicBlockData) == sizeof(int) * 3,
              "BasicBlockData should be a small object.");

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer
//
CodeBuffer::CodeBuffer(Zone* zone)
    : block_data_map_(zone),
      // TODO(eval1749) We should provide hint for size of |bytes_| to reduce
      // number of re-allocation of internal buffer.
      bytes_(zone),
      code_size_(0),
      code_values_(zone),
      current_block_data_(nullptr),
      zone_(zone) {
}

void CodeBuffer::AssociateValue(Value value) {
  DCHECK(current_block_data_);
  code_values_.push_back(CodeValue(buffer_size(), value));
}

void CodeBuffer::Finish(Factory* factory,
                        const Function* function,
                        api::MachineCodeBuilder* builder) {
  // TODO(eval1749) Fix code references, e.g. branches, indirect jumps, etc.
  // TODO(eval1749) shorten jumps.
  builder->PrepareCode(code_size_);
  for (auto const block : function->basic_blocks()) {
    auto const it = block_data_map_.find(block);
    DCHECK(it != block_data_map_.end());
    auto const data = it->second;
    builder->EmitCode(bytes_.data() + data->buffer_offset, data->code_length);
  }
  ValueEmitter value_emitter(factory, builder);
  for (auto const code_value : code_values_)
    value_emitter.Emit(code_value.code_offset, code_value.value);
  builder->FinishCode();
}

void CodeBuffer::Emit16(int value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
  bytes_.push_back(static_cast<uint8_t>(value >> 8));
}

// Emit |value| in little endian, LSB to MSB
void CodeBuffer::Emit32(uint32_t value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
  bytes_.push_back(static_cast<uint8_t>(value >> 8));
  bytes_.push_back(static_cast<uint8_t>(value >> 16));
  bytes_.push_back(static_cast<uint8_t>(value >> 24));
}

void CodeBuffer::Emit64(uint64_t value) {
  DCHECK(current_block_data_);
  Emit32(static_cast<int32_t>(value));
  Emit32(static_cast<int32_t>(value >> 32));
}

void CodeBuffer::Emit8(int value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
}

void CodeBuffer::EndBasicBlock() {
  DCHECK(current_block_data_);
  auto const data = current_block_data_;
  current_block_data_ = nullptr;
  data->code_length = buffer_size() - data->buffer_offset;
  code_size_ += data->code_length;
}

void CodeBuffer::StartBasicBlock(const BasicBlock* basic_block) {
  DCHECK(!current_block_data_);
  auto const data = new (zone_) BasicBlockData(buffer_size());
  block_data_map_[basic_block] = data;
  current_block_data_ = data;
}

}  // namespace lir
}  // namespace elang
