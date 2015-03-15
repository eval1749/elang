// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_CODE_BUFFER_H_
#define ELANG_LIR_EMITTERS_CODE_BUFFER_H_

#include <unordered_map>
#include <vector>

#include "base/basictypes.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace api {
class MachineCodeBuilder;
}
namespace lir {

class BasicBlock;
class Factory;
class Function;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// CodeBuffer
//
class ELANG_LIR_EXPORT CodeBuffer final : public ZoneOwner {
 public:
  struct ELANG_LIR_EXPORT Jump {
    int opcode;
    int opcode_size;
    int operand_size;

    Jump(int opcode, int opcode_size, int operand_size);
    int size() const { return opcode_size + operand_size; }
  };

  explicit CodeBuffer(const Function* function);
  ~CodeBuffer() = default;

  // Associate |callee| to call site at current offset.
  void AssociateCallSite(base::StringPiece16 callee);

  // Associate |value| to current offset.
  void AssociateValue(Value value);

  // Tells code emission is finished.
  void Finish(const Factory* factory, api::MachineCodeBuilder* builder);
  void Emit16(int value);
  void Emit32(uint32_t value);
  void Emit64(uint64_t value);
  void Emit8(int value);
  void EmitJump(const Jump& long_jump,
                const Jump& short_jump,
                BasicBlock* basic_block);
  void EndBasicBlock();
  void StartBasicBlock(const BasicBlock* basic_block);

 private:
  class BasicBlockData;
  class CallSite;
  class CodeBlock;
  class CodeLocation;
  class JumpData;
  class JumpResolver;
  class ValueInCode;

  int buffer_size() const { return static_cast<int>(bytes_.size()); }

  void Patch8(int buffer_offset, int value);
  void Patch32(int buffer_offset, int value);
  void PatchJump(const JumpData* jump_data);
  void RelocateAfter(int code_offset, int delta);

  std::vector<CodeBlock*> code_blocks_;
  std::unordered_map<const BasicBlock*, BasicBlockData*> block_data_map_;
  std::vector<uint8_t> bytes_;
  std::vector<CallSite*> call_site_list_;
  int code_size_;
  BasicBlockData* current_block_data_;
  std::vector<JumpData*> jump_data_list_;
  std::vector<ValueInCode*> value_in_code_list_;

  DISALLOW_COPY_AND_ASSIGN(CodeBuffer);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_CODE_BUFFER_H_
