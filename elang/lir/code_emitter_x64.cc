// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/macros.h"
#include "elang/api/machine_code_builder.h"
#include "elang/base/zone.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/code_emitter.h"
#include "elang/lir/factory.h"
#include "elang/lir/instructions.h"
#include "elang/lir/instruction_visitor.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_visitor.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {
//////////////////////////////////////////////////////////////////////
//
// BasicBlockData
//
struct BasicBlockData : ZoneAllocated {
  int buffer_offset;
  int code_length;
  int code_offset;

  explicit BasicBlockData(int buffer_offset)
      : buffer_offset(buffer_offset),
        code_length(0),
        code_offset(buffer_offset) {}
};

static_assert(sizeof(BasicBlockData) == sizeof(int) * 3,
              "BasicBlockData should be a POD.");

//////////////////////////////////////////////////////////////////////
//
// CodeValue
//
struct CodeValue {
  int code_offset;
  Value value;

  CodeValue() : code_offset(0) {}
  CodeValue(int code_offset, Value value)
      : code_offset(code_offset), value(value) {}
};

static_assert(sizeof(CodeValue) == sizeof(int) + sizeof(Value),
              "CodeValue should be a POD.");

//////////////////////////////////////////////////////////////////////
//
// ValueEmitter
//
class ValueEmitter : private LiteralVisitor {
 public:
  ValueEmitter(Factory* factory, api::MachineCodeBuilder* builder);
  ~ValueEmitter() = default;

  void Emit(int offset, Value value);

 private:
// LiteralVisitor
#define V(Name, ...) void Visit##Name(Name* type) final;
  FOR_EACH_LIR_LITERAL(V)
#undef V

  api::MachineCodeBuilder* const builder_;
  Factory* factory_;
  int code_offset_;

  DISALLOW_COPY_AND_ASSIGN(ValueEmitter);
};

ValueEmitter::ValueEmitter(Factory* factory, api::MachineCodeBuilder* builder)
    : builder_(builder), code_offset_(-1), factory_(factory) {
}

void ValueEmitter::Emit(int code_offset, Value value) {
  DCHECK_GE(code_offset, 0);
  DCHECK_EQ(code_offset_, -1);
  code_offset_ = code_offset;
  switch (value.kind) {
    case Value::Kind::Immediate:
      builder_->SetInt32(code_offset, value.data);
      break;
    case Value::Kind::Literal:
      factory_->GetLiteral(value)->Accept(this);
      break;
    default:
      NOTREACHED() << "Unexpected value: " << value;
      break;
  }
  code_offset_ = -1;
}

// LiteralVisitor
void ValueEmitter::VisitBasicBlock(BasicBlock* literal) {
  DCHECK(literal);
  NOTREACHED() << "NYI: BasicBlock literal in code emitter";
}

void ValueEmitter::VisitFunction(Function* literal) {
  DCHECK(literal);
  NOTREACHED() << "NYI: Function literal in code emitter";
}

void ValueEmitter::VisitFloat32Literal(Float32Literal* literal) {
  builder_->SetFloat32(code_offset_, literal->data());
}

void ValueEmitter::VisitFloat64Literal(Float64Literal* literal) {
  builder_->SetFloat64(code_offset_, literal->data());
}

void ValueEmitter::VisitInt32Literal(Int32Literal* literal) {
  builder_->SetInt32(code_offset_, literal->data());
}

void ValueEmitter::VisitInt64Literal(Int64Literal* literal) {
  builder_->SetInt64(code_offset_, literal->data());
}

void ValueEmitter::VisitStringLiteral(StringLiteral* literal) {
  builder_->SetString(code_offset_, literal->data());
}

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer
//
class CodeBuffer final {
 public:
  explicit CodeBuffer(Zone* zone);
  ~CodeBuffer() = default;

  void AssociateValue(Value value);
  void Finish(Factory* factory,
              const Function* function,
              api::MachineCodeBuilder* builder);
  void Emit8(int value);
  void Emit32(uint32_t value);
  void Emit64(uint64_t value);
  void EndBasicBlock();
  void StartBasicBlock(const BasicBlock* basic_block);

 private:
  int buffer_size() const { return static_cast<int>(bytes_.size()); }

  ZoneUnorderedMap<const BasicBlock*, BasicBlockData*> block_data_map_;
  ZoneVector<uint8_t> bytes_;
  int code_size_;
  ZoneVector<CodeValue> code_values_;
  BasicBlockData* current_block_data_;
  Zone* const zone_;

  DISALLOW_COPY_AND_ASSIGN(CodeBuffer);
};

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

void CodeBuffer::Emit8(int value) {
  DCHECK(current_block_data_);
  bytes_.push_back(static_cast<uint8_t>(value));
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

//////////////////////////////////////////////////////////////////////
//
// InstructionEmitter
//
class InstructionEmitter final : private InstructionVisitor {
 public:
  explicit InstructionEmitter(CodeBuffer* code_buffer);
  ~InstructionEmitter() final = default;

  void Process(const Instruction* instruction);

 private:
  void Emit32(uint32_t value);
  void Emit64(uint64_t value);
  void Emit8(int value);
  void EmitOpcode(isa::Opcode opcode);
  void EmitOperand(Value value);

// InstructionVisitor
#define V(Name, ...) void Visit##Name(Name##Instruction* instruction) final;
  FOR_EACH_LIR_INSTRUCTION(V)
#undef V

  CodeBuffer* const code_buffer_;

  DISALLOW_COPY_AND_ASSIGN(InstructionEmitter);
};

InstructionEmitter::InstructionEmitter(CodeBuffer* code_buffer)
    : code_buffer_(code_buffer) {
}

void InstructionEmitter::Emit8(int value) {
  code_buffer_->Emit8(value);
}

void InstructionEmitter::Emit32(uint32_t value) {
  code_buffer_->Emit32(value);
}

void InstructionEmitter::Emit64(uint64_t value) {
  code_buffer_->Emit64(value);
}

void InstructionEmitter::EmitOpcode(isa::Opcode opcode) {
  auto const value = static_cast<uint32_t>(opcode);
  DCHECK_LT(value, 1u << 24);
  if (value > 0xFFFF)
    Emit8(value >> 16);
  if (value > 0xFF)
    Emit8(value >> 8);
  Emit8(value);
}

void InstructionEmitter::EmitOperand(Value value) {
  code_buffer_->AssociateValue(value);
  Emit32(0);
}

void InstructionEmitter::Process(const Instruction* instruction) {
  const_cast<Instruction*>(instruction)->Accept(this);
}

// InstructionVisitor
void InstructionEmitter::VisitCall(CallInstruction* instruction) {
  EmitOpcode(instruction->opcode());
  EmitOperand(instruction->inputs()[0]);
}

void InstructionEmitter::VisitEntry(EntryInstruction* instruction) {
  __assume(instruction);
}

void InstructionEmitter::VisitExit(ExitInstruction* instruction) {
  __assume(instruction);
}

void InstructionEmitter::VisitJump(JumpInstruction* instruction) {
  __assume(instruction);
}

void InstructionEmitter::VisitLoad(LoadInstruction* instruction) {
  __assume(instruction);
}

void InstructionEmitter::VisitRet(RetInstruction* instruction) {
  EmitOpcode(instruction->opcode());
}

}  // namespace

void CodeEmitter::Process(const Function* function) {
  Zone zone;
  CodeBuffer code_buffer(&zone);
  // Generate codes
  {
    InstructionEmitter emitter(&code_buffer);
    for (auto const block : function->basic_blocks()) {
      code_buffer.StartBasicBlock(block);
      for (auto const instruction : block->instructions())
        emitter.Process(instruction);
      code_buffer.EndBasicBlock();
    }
  }
  code_buffer.Finish(factory_, function, builder_);
}

}  // namespace lir
}  // namespace elang
