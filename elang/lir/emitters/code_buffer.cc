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

namespace {
// TODO(eval1749) We should move |Is8Bit()| to another place to share code.
bool Is8Bit(int data) {
  return (data & 255) == data;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::CodeLocation
//
class CodeBuffer::CodeLocation : public ZoneAllocated {
 public:
  CodeLocation(int buffer_offset, int code_offset);
  CodeLocation();
  ~CodeLocation();

  int buffer_offset() const { return buffer_offset_; }
  int code_offset() const { return code_offset_; }

  void Relocate(int delta);
  void Start(int buffer_offset, int code_offset);

 private:
  int buffer_offset_;
  int code_offset_;

  DISALLOW_COPY_AND_ASSIGN(CodeLocation);
};

CodeBuffer::CodeLocation::CodeLocation(int buffer_offset, int code_offset)
    : buffer_offset_(buffer_offset), code_offset_(code_offset) {
}

CodeBuffer::CodeLocation::CodeLocation()
    : buffer_offset_(-1), code_offset_(-1) {
}

CodeBuffer::CodeLocation::~CodeLocation() {
  NOTREACHED();
}

// Relocate this |CodeLocation|. |delta| can be negative for code alignment
// adjustment.
void CodeBuffer::CodeLocation::Relocate(int delta) {
  code_offset_ += delta;
}

void CodeBuffer::CodeLocation::Start(int buffer_offset, int code_offset) {
  DCHECK_EQ(buffer_offset_, -1);
  DCHECK_EQ(code_offset_, -1);
  buffer_offset_ = buffer_offset;
  code_offset_ = code_offset;
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::BasicBlockData
//
class CodeBuffer::BasicBlockData : public CodeLocation {
 public:
  BasicBlockData();
  ~BasicBlockData() = delete;

  int code_length() const { return code_length_; }
  void set_code_length(int new_length);

  void RelocateBlock(int delta);

 private:
  int code_length_;

  DISALLOW_COPY_AND_ASSIGN(BasicBlockData);
};

CodeBuffer::BasicBlockData::BasicBlockData() : code_length_(-1) {
}

void CodeBuffer::BasicBlockData::set_code_length(int new_length) {
  code_length_ = new_length;
}

void CodeBuffer::BasicBlockData::RelocateBlock(int delta) {
  Relocate(delta);
  code_length_ += delta;
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::JumpData
//
class CodeBuffer::JumpData final : public CodeLocation {
 public:
  JumpData(int buffer_offset,
           int code_offset,
           const Jump& long_jump,
           const Jump& short_jump,
           BasicBlockData* target_block);

  ~JumpData() = delete;

  bool is_long_jump() const { return is_long_jump_; }
  const Jump& jump() const { return is_long_jump_ ? long_jump_ : short_jump_; }
  int target_code_offset() const;

  bool IsCrossing(int code_offset) const;
  int RelativeOffset() const;
  const Jump& UseLongJump();

 private:
  friend class JumpResolver;

  bool is_long_jump_;
  const Jump long_jump_;
  const Jump short_jump_;
  const BasicBlockData* const target_block_;

  DISALLOW_COPY_AND_ASSIGN(JumpData);
};

CodeBuffer::JumpData::JumpData(int buffer_offset,
                               int code_offset,
                               const Jump& long_jump,
                               const Jump& short_jump,
                               BasicBlockData* target_block)
    : CodeLocation(buffer_offset, code_offset),
      is_long_jump_(false),
      long_jump_(long_jump),
      short_jump_(short_jump),
      target_block_(target_block) {
  DCHECK_NE(long_jump_.opcode, short_jump.opcode);
  DCHECK_GT(long_jump_.size(), short_jump_.size());
}

bool CodeBuffer::JumpData::IsCrossing(int ref_code_offset) const {
  if (code_offset() < ref_code_offset) {
    //    jump target
    //    -- |code_offset| --
    //  target:
    return target_block_->code_offset() >= ref_code_offset;
  }

  // target:
  //    -- |code_offset|
  //    jump target
  return target_block_->code_offset() < ref_code_offset;
}

int CodeBuffer::JumpData::RelativeOffset() const {
  return target_block_->code_offset() - code_offset();
}

const CodeBuffer::Jump& CodeBuffer::JumpData::UseLongJump() {
  DCHECK(!is_long_jump_);
  is_long_jump_ = true;
  return long_jump_;
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::JumpResolver
//
class CodeBuffer::JumpResolver final {
 public:
  explicit JumpResolver(CodeBuffer* code_buffer);
  ~JumpResolver() = default;

  void Run();

 private:
  void AnalyzeJump(JumpData* data);
  void SetJump(const JumpData* data);
  void UpdateWorkSet(int code_offset);

  CodeBuffer* const code_buffer_;
  std::unordered_set<JumpData*> work_set_;

  DISALLOW_COPY_AND_ASSIGN(JumpResolver);
};

CodeBuffer::JumpResolver::JumpResolver(CodeBuffer* code_buffer)
    : code_buffer_(code_buffer) {
}

void CodeBuffer::JumpResolver::AnalyzeJump(JumpData* data) {
  if (data->is_long_jump())
    return;
  auto const relative_offset = data->RelativeOffset();
  if (Is8Bit(relative_offset))
    return;
  UpdateWorkSet(data->code_offset());
  auto const short_jump = data->jump();
  auto const long_jump = data->UseLongJump();
  code_buffer_->RelocateAfter(data->code_offset(),
                              long_jump.size() - short_jump.size());
}

void CodeBuffer::JumpResolver::Run() {
  work_set_.insert(code_buffer_->jump_data_list_.begin(),
                   code_buffer_->jump_data_list_.end());
  while (!work_set_.empty()) {
    auto const data = *work_set_.begin();
    AnalyzeJump(data);
    work_set_.erase(data);
  }
}

void CodeBuffer::JumpResolver::UpdateWorkSet(int code_offset) {
  for (auto const data : code_buffer_->jump_data_list_) {
    if (work_set_.count(data))
      continue;
    if (!data->IsCrossing(code_offset))
      continue;
    work_set_.insert(data);
  }
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::ValueInCode represents reference to |Value| in code buffer.
//
class CodeBuffer::ValueInCode : public CodeLocation {
 public:
  ValueInCode(int buffer_offset, int code_offset, Value value);
  ~ValueInCode() = delete;

  Value value() const { return value_; }

 private:
  const Value value_;

  DISALLOW_COPY_AND_ASSIGN(ValueInCode);
};

CodeBuffer::ValueInCode::ValueInCode(int buffer_offset,
                                     int code_offset,
                                     Value value)
    : CodeLocation(buffer_offset, code_offset), value_(value) {
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer
//
// TODO(eval1749) We should provide hint for size of |bytes_| to reduce
// number of re-allocation of internal buffer.
CodeBuffer::CodeBuffer(const Function* function)
    : code_size_(0), current_block_data_(nullptr) {
  for (auto const block : function->basic_blocks()) {
    auto const data = new (zone()) BasicBlockData();
    block_data_list_.push_back(data);
    block_data_map_[block] = data;
  }
}

void CodeBuffer::AssociateValue(Value value) {
  DCHECK(current_block_data_);
  value_in_code_list_.push_back(
      new (zone()) ValueInCode(buffer_size(), code_size_, value));
}

void CodeBuffer::Finish(const Factory* factory,
                        api::MachineCodeBuilder* builder) {
  // TODO(eval1749) Fix code references, e.g. branches, indirect jumps, etc.
  JumpResolver(this).Run();
  for (auto const jump_data : jump_data_list_)
    PatchJump(jump_data);
  builder->PrepareCode(code_size_);
  for (auto const data : block_data_list_) {
    DCHECK_GE(data->buffer_offset(), 0);
    DCHECK_GE(data->code_offset(), 0);
    // TODO(eval1749) Insert target specific NOP instructions for code
    // alignment.
    if (!data->code_length())
      continue;
    DCHECK_GT(data->code_length(), 0);
    builder->EmitCode(bytes_.data() + data->buffer_offset(),
                      data->code_length());
  }
  ValueEmitter value_emitter(factory, builder);
  for (auto const value_in_code : value_in_code_list_)
    value_emitter.Emit(value_in_code->code_offset(), value_in_code->value());
  builder->FinishCode();
}

void CodeBuffer::Emit16(int value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
  bytes_.push_back(static_cast<uint8_t>(value >> 8));
  code_size_ += 2;
}

// Emit |value| in little endian, LSB to MSB
void CodeBuffer::Emit32(uint32_t value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
  bytes_.push_back(static_cast<uint8_t>(value >> 8));
  bytes_.push_back(static_cast<uint8_t>(value >> 16));
  bytes_.push_back(static_cast<uint8_t>(value >> 24));
  code_size_ += 4;
}

void CodeBuffer::Emit64(uint64_t value) {
  DCHECK(current_block_data_);
  Emit32(static_cast<int32_t>(value));
  Emit32(static_cast<int32_t>(value >> 32));
}

void CodeBuffer::Emit8(int value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
  ++code_size_;
}

// Emit jump into code buffer. We assume all jump are short jump but we reserve
// room for long jump. In |Finish()| function, we change short jumps to long
// jumps if needed.
void CodeBuffer::EmitJump(const Jump& long_jump,
                          const Jump& short_jump,
                          BasicBlock* target_block) {
  DCHECK(current_block_data_);
  DCHECK_NE(long_jump.opcode, short_jump.opcode);
  DCHECK_GT(long_jump.size(), short_jump.size());
  jump_data_list_.push_back(
      new (zone()) JumpData(buffer_size(), code_size_, long_jump, short_jump,
                            block_data_map_[target_block]));
  // Reserve buffer for long jump.
  bytes_.resize(buffer_size() + long_jump.size());
  code_size_ += short_jump.size();
}

void CodeBuffer::EndBasicBlock() {
  DCHECK(current_block_data_);
  auto const data = current_block_data_;
  current_block_data_ = nullptr;
  data->set_code_length(code_size_ - data->code_offset());
}

void CodeBuffer::RelocateAfter(int ref_code_offset, int delta) {
  DCHECK_GT(delta, 0);
  for (auto it = jump_data_list_.rbegin(); it != jump_data_list_.rbegin();
       ++it) {
    auto const jump_data = *it;
    if (jump_data->code_offset() < ref_code_offset)
      continue;
    jump_data->Relocate(delta);
  }
  for (auto it = block_data_list_.rbegin(); it != block_data_list_.rend();
       ++it) {
    auto const block_data = *it;
    if (block_data->code_offset() < ref_code_offset)
      break;
    block_data->RelocateBlock(delta);
  }
  for (auto it = value_in_code_list_.rbegin(); it != value_in_code_list_.rend();
       ++it) {
    auto const value_in_code = *it;
    if (value_in_code->code_offset() < ref_code_offset)
      break;
    value_in_code->Relocate(delta);
  }
}

void CodeBuffer::Patch8(int buffer_offset, int value) {
  bytes_[buffer_offset] = static_cast<uint8_t>(value);
}

void CodeBuffer::Patch32(int buffer_offset, int value) {
  bytes_[buffer_offset] = static_cast<uint8_t>(value >> 24);
  bytes_[buffer_offset + 1] = static_cast<uint8_t>(value >> 16);
  bytes_[buffer_offset + 2] = static_cast<uint8_t>(value >> 8);
  bytes_[buffer_offset + 3] = static_cast<uint8_t>(value);
}

void CodeBuffer::PatchJump(const JumpData* jump_data) {
  auto const jump = jump_data->jump();
  auto offset = jump_data->buffer_offset();

  // Set opcode of jump instruction
  auto opcode = jump.opcode;
  for (auto count = jump.opcode_size; count; --count) {
    Patch8(offset, opcode);
    opcode >>= 8;
    ++offset;
  }

  // Set operand of jump instruction
  auto const relative_offset = jump_data->RelativeOffset();
  if (jump.operand_size == 4) {
    Patch32(offset, relative_offset);
    return;
  }
  if (jump.operand_size == 1) {
    DCHECK(Is8Bit(relative_offset));
    Patch8(offset, relative_offset);
    return;
  }
  NOTREACHED() << "Unsupported relative offset size" << jump.operand_size;
}

void CodeBuffer::StartBasicBlock(const BasicBlock* block) {
  DCHECK(!current_block_data_);
  auto const it = block_data_map_.find(block);
  DCHECK(it != block_data_map_.end());
  auto const data = it->second;
  // TODO(eval1749) If one of incoming edge is back edge, we should align
  // code offset of this block in target's code cache alignment, e.g. 16 bytes.
  data->Start(buffer_size(), code_size_);
  current_block_data_ = it->second;
}

}  // namespace lir
}  // namespace elang
