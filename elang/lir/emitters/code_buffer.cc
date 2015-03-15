// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/code_buffer.h"

#include "base/logging.h"
#include "elang/api/machine_code_builder.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/lir/emitters/value_emitter.h"
#include "elang/lir/factory.h"
#include "elang/lir/literals.h"

namespace elang {
namespace lir {

namespace {
// TODO(eval1749) We should move |Is8Bit()| to another place to share code.
bool Is8Bit(int data) {
  return data >= -128 && data <= 127;
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::CodeLocation
//
class CodeBuffer::CodeLocation : public Castable, public ZoneAllocated {
  DECLARE_CASTABLE_CLASS(CodeLocation, Castable);

 public:
  CodeLocation(int buffer_offset, int code_offset);
  CodeLocation();
  ~CodeLocation() override;

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
// CodeBuffer::CallSite represents reference to |Value| in code buffer.
//
class CodeBuffer::CallSite final : public CodeLocation {
  DECLARE_CASTABLE_CLASS(CallSite, CodeLocation);

 public:
  CallSite(int buffer_offset, int code_offset, base::StringPiece16 callee);
  ~CallSite() override = default;

  base::StringPiece16 callee() const { return callee_; }

 private:
  const base::StringPiece16 callee_;

  DISALLOW_COPY_AND_ASSIGN(CallSite);
};

CodeBuffer::CallSite::CallSite(int buffer_offset,
                               int code_offset,
                               base::StringPiece16 callee)
    : CodeLocation(buffer_offset, code_offset), callee_(callee) {
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::CodeBlock
//
class CodeBuffer::CodeBlock : public CodeLocation {
  DECLARE_CASTABLE_CLASS(CodeBlock, CodeLocation);

 public:
  int code_length() const { return code_length_; }

 protected:
  CodeBlock(int buffer_offset, int code_offset, int code_length);
  CodeBlock();
  ~CodeBlock() override = default;

  void set_code_length(int new_length);

 private:
  int code_length_;

  DISALLOW_COPY_AND_ASSIGN(CodeBlock);
};

CodeBuffer::CodeBlock::CodeBlock(int buffer_offset,
                                 int code_offset,
                                 int code_length)
    : CodeLocation(buffer_offset, code_offset), code_length_(code_length) {
}

CodeBuffer::CodeBlock::CodeBlock() : code_length_(-1) {
}

void CodeBuffer::CodeBlock::set_code_length(int new_length) {
  code_length_ = new_length;
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::BasicBlockData
//
class CodeBuffer::BasicBlockData final : public CodeBlock {
  DECLARE_CASTABLE_CLASS(BasicBlockData, CodeBlock);

 public:
  BasicBlockData() = default;
  ~BasicBlockData() final = default;

  void EndBasicBlock(int code_offset);

 private:
  DISALLOW_COPY_AND_ASSIGN(BasicBlockData);
};

void CodeBuffer::BasicBlockData::EndBasicBlock(int code_size) {
  auto const new_code_length = code_size - code_offset();
  DCHECK_GE(new_code_length, 0);
  set_code_length(new_code_length);
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::Jump
//
CodeBuffer::Jump::Jump(int opcode, int opcode_size, int operand_size)
    : opcode(opcode), opcode_size(opcode_size), operand_size(operand_size) {
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer::JumpSite
//
class CodeBuffer::JumpSite final : public CodeBlock {
  DECLARE_CASTABLE_CLASS(JumpSite, CodeBlock);

 public:
  JumpSite(int buffer_offset,
           int code_offset,
           const Jump& long_jump,
           const Jump& short_jump,
           BasicBlockData* target_block);

  ~JumpSite() final = default;

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

  DISALLOW_COPY_AND_ASSIGN(JumpSite);
};

CodeBuffer::JumpSite::JumpSite(int buffer_offset,
                               int code_offset,
                               const Jump& long_jump,
                               const Jump& short_jump,
                               BasicBlockData* target_block)
    : CodeBlock(buffer_offset, code_offset, short_jump.size()),
      is_long_jump_(false),
      long_jump_(long_jump),
      short_jump_(short_jump),
      target_block_(target_block) {
  DCHECK_NE(long_jump_.opcode, short_jump.opcode);
  DCHECK_GT(long_jump_.size(), short_jump_.size());
}

bool CodeBuffer::JumpSite::IsCrossing(int ref_code_offset) const {
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

int CodeBuffer::JumpSite::RelativeOffset() const {
  return target_block_->code_offset() - (code_offset() + jump().size());
}

const CodeBuffer::Jump& CodeBuffer::JumpSite::UseLongJump() {
  DCHECK(!is_long_jump_);
  is_long_jump_ = true;
  set_code_length(long_jump_.size());
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
  void AnalyzeJump(JumpSite* data);
  void SetJump(const JumpSite* data);
  void UpdateWorkSet(int code_offset);

  CodeBuffer* const code_buffer_;
  std::unordered_set<JumpSite*> work_set_;

  DISALLOW_COPY_AND_ASSIGN(JumpResolver);
};

CodeBuffer::JumpResolver::JumpResolver(CodeBuffer* code_buffer)
    : code_buffer_(code_buffer) {
}

void CodeBuffer::JumpResolver::AnalyzeJump(JumpSite* data) {
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
  work_set_.insert(code_buffer_->jump_sites_.begin(),
                   code_buffer_->jump_sites_.end());
  while (!work_set_.empty()) {
    auto const data = *work_set_.begin();
    AnalyzeJump(data);
    work_set_.erase(data);
  }
}

void CodeBuffer::JumpResolver::UpdateWorkSet(int code_offset) {
  for (auto const data : code_buffer_->jump_sites_) {
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
class CodeBuffer::ValueInCode final : public CodeLocation {
  DECLARE_CASTABLE_CLASS(ValueInCode, CodeLocation);

 public:
  ValueInCode(int buffer_offset, int code_offset, Value value);
  ~ValueInCode() final = default;

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
  for (auto const block : function->basic_blocks())
    block_data_map_[block] = new (zone()) BasicBlockData();
}

void CodeBuffer::AssociateCallSite(base::StringPiece16 callee) {
  DCHECK(current_block_data_);
  code_locations_.push_back(
      new (zone()) CallSite(buffer_size(), code_size_, callee));
}

void CodeBuffer::AssociateValue(Value value) {
  DCHECK(current_block_data_);
  code_locations_.push_back(
      new (zone()) ValueInCode(buffer_size(), code_size_, value));
}

void CodeBuffer::Finish(const Factory* factory,
                        api::MachineCodeBuilder* builder) {
  // TODO(eval1749) Fix code references, e.g. branches, indirect jumps, etc.
  JumpResolver(this).Run();
  for (auto const jump_site : jump_sites_)
    PatchJump(jump_site);
  builder->PrepareCode(code_size_);

  ValueEmitter value_emitter(factory, builder);

  auto code_offset = 0;
  for (auto const code_location : code_locations_) {
    if (auto const call_site = code_location->as<CallSite>()) {
      builder->SetCallSite(call_site->code_offset(), call_site->callee());
      continue;
    }
    if (auto const value_in_code = code_location->as<ValueInCode>()) {
      value_emitter.Emit(value_in_code->code_offset(), value_in_code->value());
      continue;
    }

    auto const code_block = code_location->as<CodeBlock>();
    if (!code_block)
      continue;
    DCHECK_GE(code_block->buffer_offset(), 0);
    DCHECK_EQ(code_offset, code_block->code_offset());
    code_offset += code_block->code_length();
    // TODO(eval1749) Insert target specific NOP instructions for code
    // alignment.
    if (!code_block->code_length())
      continue;
    DCHECK_GT(code_block->code_length(), 0);
    builder->EmitCode(bytes_.data() + code_block->buffer_offset(),
                      code_block->code_length());
  }

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
  EndBasicBlock();
  DCHECK_NE(long_jump.opcode, short_jump.opcode);
  DCHECK_GT(long_jump.size(), short_jump.size());
  auto const jump_site =
      new (zone()) JumpSite(buffer_size(), code_size_, long_jump, short_jump,
                            block_data_map_[target_block]);
  code_locations_.push_back(jump_site);
  jump_sites_.push_back(jump_site);
  // Reserve buffer for long jump.
  bytes_.resize(buffer_size() + long_jump.size());
  code_size_ += short_jump.size();
}

void CodeBuffer::EndBasicBlock() {
  if (!current_block_data_)
    return;
  auto const block_data = current_block_data_;
  current_block_data_ = nullptr;
  block_data->EndBasicBlock(code_size_);
}

void CodeBuffer::RelocateAfter(int ref_code_offset, int delta) {
  DCHECK_GT(delta, 0);
  code_size_ += delta;
  for (auto it = code_locations_.rbegin(); it != code_locations_.rend(); ++it) {
    auto const code_location = *it;
    if (code_location->code_offset() <= ref_code_offset)
      break;
    code_location->Relocate(delta);
  }
}

void CodeBuffer::Patch8(int buffer_offset, int value) {
  bytes_[buffer_offset] = static_cast<uint8_t>(value);
}

void CodeBuffer::Patch32(int buffer_offset, int value) {
  bytes_[buffer_offset] = static_cast<uint8_t>(value);
  bytes_[buffer_offset + 1] = static_cast<uint8_t>(value >> 8);
  bytes_[buffer_offset + 2] = static_cast<uint8_t>(value >> 16);
  bytes_[buffer_offset + 3] = static_cast<uint8_t>(value >> 24);
}

void CodeBuffer::PatchJump(const JumpSite* jump_site) {
  auto const jump = jump_site->jump();
  DCHECK_LE(jump.opcode_size, 2);
  auto buffer_offset = jump_site->buffer_offset();

  // Set opcode of jump instruction
  auto opcode = jump.opcode;
  if (jump.opcode_size == 2) {
    Patch8(buffer_offset, opcode >> 8);
    ++buffer_offset;
  }
  Patch8(buffer_offset, opcode);
  ++buffer_offset;

  // Set operand of jump instruction
  auto const relative_offset = jump_site->RelativeOffset();
  if (jump.operand_size == 4) {
    DCHECK(!Is8Bit(relative_offset));
    Patch32(buffer_offset, relative_offset);
    return;
  }
  if (jump.operand_size == 1) {
    DCHECK(Is8Bit(relative_offset));
    Patch8(buffer_offset, relative_offset);
    return;
  }
  NOTREACHED() << "Unsupported relative offset size" << jump.operand_size;
}

void CodeBuffer::StartBasicBlock(const BasicBlock* block) {
  DCHECK(!current_block_data_);
  auto const it = block_data_map_.find(block);
  DCHECK(it != block_data_map_.end());
  auto const block_data = it->second;
  // TODO(eval1749) If one of incoming edge is back edge, we should align
  // code offset of this block in target's code cache alignment, e.g. 16 bytes.
  block_data->Start(buffer_size(), code_size_);
  current_block_data_ = block_data;
  code_locations_.push_back(block_data);
}

}  // namespace lir
}  // namespace elang
