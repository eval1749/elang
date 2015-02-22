// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EMITTERS_CODE_BUFFER_H_
#define ELANG_LIR_EMITTERS_CODE_BUFFER_H_

#include "base/basictypes.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"
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
class CodeBuffer final {
 public:
  struct BasicBlockData;

  // CodeValue represents reference to |Value| in code buffer.
  struct CodeValue {
    int code_offset;
    Value value;

    CodeValue() : code_offset(0) {}
    CodeValue(int code_offset, Value value)
        : code_offset(code_offset), value(value) {}
  };

  static_assert(sizeof(CodeValue) == sizeof(int) + sizeof(Value),
                "CodeValue should be small object.");

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

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EMITTERS_CODE_BUFFER_H_
