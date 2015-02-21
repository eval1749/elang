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
#include "elang/lir/isa_x64.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_visitor.h"
#include "elang/lir/opcodes_x64.h"
#include "elang/lir/target_x64.h"
#include "elang/lir/target.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

namespace {

using isa::Mod;
using isa::Register;
using isa::Rm;
using isa::Rex;
using isa::Scale;
using isa::Tttn;

bool Is8Bit(int data) {
  return (data & 255) == data;
}

bool Is32Bit(int64_t data) {
  return (data & static_cast<uint32_t>(-1)) == data;
}

Value To32bitRegister(Value reg) {
  DCHECK(reg.is_physical());
  DCHECK_EQ(ValueSize::Size64, reg.size);
  return Value(reg.is_integer() ? Value::Type::Integer : Value::Type::Float,
               ValueSize::Size32, Value::Kind::PhysicalRegister, reg.data & 15);
}

isa::Register ToRegister(Value reg) {
  DCHECK(reg.is_physical());
  if (reg.is_float()) {
    if (reg.size == ValueSize::Size32)
      return static_cast<Register>(isa::XMM0S + (reg.data & 15));
    DCHECK_EQ(ValueSize::Size64, reg.size);
    return static_cast<Register>(isa::XMM0D + (reg.data & 15));
  }
  switch (reg.size) {
    case ValueSize::Size8:
      return static_cast<Register>(isa::AL + (reg.data & 15));
    case ValueSize::Size16:
      return static_cast<Register>(isa::AX + (reg.data & 15));
    case ValueSize::Size32:
      return static_cast<Register>(isa::EAX + (reg.data & 15));
    case ValueSize::Size64:
      return static_cast<Register>(isa::RAX + (reg.data & 15));
  }
  NOTREACHED() << "Unknown size: " << reg.size;
  return static_cast<Register>(0);
}

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
  void Emit16(int value);
  void Emit32(uint32_t value);
  void Emit64(uint64_t value);
  void Emit8(int value);
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

//////////////////////////////////////////////////////////////////////
//
// InstructionEmitter
//
class InstructionEmitter final : private InstructionVisitor {
 public:
  InstructionEmitter(const Factory* factory, CodeBuffer* code_buffer);
  ~InstructionEmitter() final = default;

  void Process(const Instruction* instruction);

 private:
  void Emit16(int value);
  void Emit32(uint32_t value);
  void Emit64(uint64_t value);
  void Emit8(int value);
  void EmitModRm(Mod mod, Register reg, Register rm);
  void EmitModRm(Mod mod, Register reg, Rm rm);
  void EmitModRm(Register reg, Value input);
  void EmitModRm(Value output, isa::OpcodeExt opext);
  void EmitModRm(Value output, Value input);
  void EmitOpcode(isa::Opcode opcode);
  void EmitOpcode(isa::Opcode opcode, Register reg);
  void EmitOperand(Value value);
  void EmitRexPrefix(Value output, Value input);
  void EmitSib(Scale scale, Register index, Register base);

  int32_t Int32ValueOf(Value literal) const;
  bool Is32BitLiteral(Value literal) const;

  // InstructionVisitor
  void VisitCall(CallInstruction* instr) final;
  void VisitCopy(CopyInstruction* instr) final;
  void VisitLiteral(LiteralInstruction* instr) final;
  void VisitRet(RetInstruction* instr) final;

  CodeBuffer* const code_buffer_;
  const Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionEmitter);
};

InstructionEmitter::InstructionEmitter(const Factory* factory,
                                       CodeBuffer* code_buffer)
    : code_buffer_(code_buffer), factory_(factory) {
}

void InstructionEmitter::Emit16(int value) {
  code_buffer_->Emit16(value);
}

void InstructionEmitter::Emit32(uint32_t value) {
  code_buffer_->Emit32(value);
}

void InstructionEmitter::Emit64(uint64_t value) {
  code_buffer_->Emit64(value);
}

void InstructionEmitter::Emit8(int value) {
  code_buffer_->Emit8(value);
}

void InstructionEmitter::EmitModRm(Mod mod, Register reg, Register rm) {
  Emit8(static_cast<int>(mod) | ((reg & 7) << 3) | (rm & 7));
}

void InstructionEmitter::EmitModRm(Mod mod, Register reg, Rm rm) {
  EmitModRm(mod, reg, static_cast<Register>(rm));
}

void InstructionEmitter::EmitModRm(Register reg, Value memory) {
  if (memory.is_frame_slot()) {
    if (!memory.data) {
      // mov reg, [rbp]
      EmitModRm(Mod::Disp0, reg, isa::RBP);
      return;
    }
    // mov reg, [rbp+n]
    if (Is8Bit(memory.data)) {
      EmitModRm(Mod::Disp8, reg, isa::RBP);
      Emit8(memory.data);
      return;
    }
    // mov reg, [rbp+n]
    EmitModRm(Mod::Disp32, reg, isa::RBP);
    Emit32(memory.data);
    return;
  }
  if (memory.is_stack_slot()) {
    if (!memory.data) {
      EmitModRm(Mod::Disp0, reg, Rm::Sib);
      EmitSib(Scale::One, isa::RSP, isa::RSP);
      return;
    }
    if (Is8Bit(memory.data)) {
      // mov reg, [rsp+disp8]
      EmitModRm(Mod::Disp8, reg, Rm::Sib);
      EmitSib(Scale::One, isa::RSP, isa::RSP);
      Emit8(memory.data);
      return;
    }
    // mov reg, [rsp+n]
    EmitModRm(Mod::Disp32, reg, Rm::Sib);
    EmitSib(Scale::One, isa::RSP, isa::RSP);
    Emit32(memory.data);
    return;
  }
  NOTREACHED() << "EmitModRm " << reg << ", " << memory;
}

void InstructionEmitter::EmitModRm(Value output, isa::OpcodeExt opext) {
  EmitModRm(output, Target::GetRegister(static_cast<Register>(opext)));
}

void InstructionEmitter::EmitModRm(Value output, Value input) {
  if (output.is_physical()) {
    auto const reg = static_cast<Register>(output.data);
    if (input.is_physical()) {
      // mov reg1, reg2
      EmitModRm(Mod::Reg, reg, static_cast<Register>(input.data));
      return;
    }
    EmitModRm(reg, input);
    return;
  }
  if (input.is_physical()) {
    EmitModRm(static_cast<Register>(input.data), output);
    return;
  }
  NOTREACHED() << "EmitModRm " << output << ", " << input;
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

void InstructionEmitter::EmitOpcode(isa::Opcode opcode, Register reg) {
  EmitOpcode(static_cast<isa::Opcode>(static_cast<int>(opcode) + (reg & 7)));
}

void InstructionEmitter::EmitOperand(Value value) {
  if (value.is_immediate()) {
    switch (value.size) {
      case ValueSize::Size8:
        Emit8(value.data);
        return;
      case ValueSize::Size16:
        Emit16(value.data);
        return;
      case ValueSize::Size32:
        Emit32(value.data);
        return;
      case ValueSize::Size64:
        Emit64(value.data);
        return;
    }
  }
  if (value.is_literal()) {
    auto const literal = factory_->GetLiteral(value);
    if (auto const i32 = literal->as<Int32Literal>())
      return Emit32(i32->data());
    if (auto const i64 = literal->as<Int64Literal>())
      return Emit64(i64->data());
  }
  code_buffer_->AssociateValue(value);
  Emit32(0);
}

void InstructionEmitter::EmitRexPrefix(Value output, Value input) {
  if (output.size == ValueSize::Size16) {
    EmitOpcode(isa::Opcode::OPDSIZ);
    return;
  }
  int rex = isa::REX;
  if (output.size == ValueSize::Size64)
    rex |= isa::REX_W;
  if (output.is_physical() && output.data >= 8)
    rex |= isa::REX_R;
  if (input.is_physical() && input.data >= 8)
    rex |= isa::REX_B;
  if (rex == isa::REX)
    return;
  Emit8(rex);
}

void InstructionEmitter::EmitSib(Scale scale, Register index, Register base) {
  Emit8(static_cast<int>(scale) | ((index & 7) << 3) | (base & 7));
}

int32_t InstructionEmitter::Int32ValueOf(Value value) const {
  if (value.is_immediate())
    return value.data;
  DCHECK(value.is_literal());
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->as<Int32Literal>())
    return i32->data();
  if (auto const i64 = literal->as<Int64Literal>())
    return static_cast<int32_t>(i64->data());
  NOTREACHED() << value << " isn't 32-bit literal";
  return 0;
}

bool InstructionEmitter::Is32BitLiteral(Value value) const {
  if (value.is_immediate())
    return true;
  if (!value.is_literal())
    return false;
  auto const literal = factory_->GetLiteral(value);
  if (auto const i32 = literal->is<Int32Literal>())
    return true;
  if (auto const i64 = literal->as<Int64Literal>())
    return Is32Bit(i64->data());
  return false;
}

void InstructionEmitter::Process(const Instruction* instr) {
  const_cast<Instruction*>(instr)->Accept(this);
}

// InstructionVisitor
void InstructionEmitter::VisitCall(CallInstruction* instr) {
  EmitOpcode(isa::Opcode::CALL_Jv);
  EmitOperand(instr->input(0));
}

void InstructionEmitter::VisitCopy(CopyInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);

  EmitRexPrefix(output, input);

  if (output.is_physical()) {
    if (output.is_integer())
      EmitOpcode(isa::Opcode::MOV_Gv_Ev);
    else if (output.size == ValueSize::Size32)
      EmitOpcode(isa::Opcode::MOVSS_Vss_Wss);
    else
      EmitOpcode(isa::Opcode::MOVSD_Vsd_Wsd);
  } else {
    if (output.is_integer())
      EmitOpcode(isa::Opcode::MOV_Ev_Gv);
    else if (output.size == ValueSize::Size32)
      EmitOpcode(isa::Opcode::MOVSS_Wss_Vss);
    else
      EmitOpcode(isa::Opcode::MOVSD_Wsd_Vsd);
  }

  EmitModRm(output, input);
}

void InstructionEmitter::VisitLiteral(LiteralInstruction* instr) {
  auto const input = instr->input(0);
  auto const output = instr->output(0);
  DCHECK_EQ(input.size, output.size);
  DCHECK_EQ(input.type, output.type);
  DCHECK(output.is_integer()) << "NYI: float " << *instr;

  if (output.is_physical() && output.size == ValueSize::Size64 &&
      Is32BitLiteral(input)) {
    auto const imm32 = Int32ValueOf(input);
    if (imm32 >= 0) {
      // 8B/r mov r32, imm32 : 8B/r imm32
      auto const reg32 = To32bitRegister(output);
      EmitRexPrefix(reg32, input);
      EmitOpcode(isa::Opcode::MOV_Ev_Iz, ToRegister(reg32));
      Emit32(imm32);
      return;
    }
    // sign extended to 64 bits to r/m64
    // C7/0 mov r/m64, imm32 : REX.W+C7 /0 imm32
    EmitRexPrefix(output, input);
    EmitOpcode(isa::Opcode::MOV_Ev_Iz);
    EmitModRm(output, isa::OpcodeExt::MOV_Ev_Iz);
    Emit32(imm32);
    return;
  }

  EmitRexPrefix(output, input);
  if (output.size == ValueSize::Size8)
    EmitOpcode(isa::Opcode::MOV_Eb_Ib);
  else
    EmitOpcode(isa::Opcode::MOV_Ev_Iz);
  EmitModRm(output, isa::OpcodeExt::MOV_Ev_Iz);
  EmitOperand(input);
}

void InstructionEmitter::VisitRet(RetInstruction* instr) {
  EmitOpcode(isa::Opcode::RET);
}

}  // namespace

void CodeEmitter::Process(const Function* function) {
  Zone zone;
  CodeBuffer code_buffer(&zone);
  // Generate codes
  {
    InstructionEmitter emitter(factory_, &code_buffer);
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
