// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>
#include <vector>

#include "elang/lir/formatters/text_formatter.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"
#include "elang/lir/literal_visitor.h"
#include "elang/lir/printable.h"
#include "elang/lir/target.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace std {
std::ostream& operator<<(std::ostream& ostream,
                         const std::vector<elang::lir::BasicBlock*>& blocks) {
  ostream << "{";
  auto separator = "";
  for (auto const block : blocks) {
    ostream << separator << *block;
    separator = ", ";
  }
  return ostream << "}";
}
}  // namespace std

namespace elang {
namespace lir {

namespace {

std::vector<BasicBlock*> SortBasicBlocks(
    const ZoneUnorderedSet<BasicBlock*>& block_set) {
  std::vector<BasicBlock*> blocks(block_set.begin(), block_set.end());
  std::sort(blocks.begin(), blocks.end(),
            [](BasicBlock* a, BasicBlock* b) { return a->id() < b->id(); });
  return blocks;
}

//////////////////////////////////////////////////////////////////////
//
// LiteralFormatter
//
class LiteralFormatter final : public LiteralVisitor {
 public:
  explicit LiteralFormatter(std::ostream* ostream);
  ~LiteralFormatter() = default;

  std::ostream& Format(const Literal* literal);

 private:
#define V(Name, ...) void Visit##Name(Name* literal) final;
  FOR_EACH_LIR_LITERAL(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(LiteralFormatter);
};

LiteralFormatter::LiteralFormatter(std::ostream* ostream) : ostream_(*ostream) {
}

std::ostream& LiteralFormatter::Format(const Literal* literal) {
  const_cast<Literal*>(literal)->Accept(this);
  return ostream_;
}

void LiteralFormatter::VisitBasicBlock(BasicBlock* block) {
  ostream_ << *block;
}

void LiteralFormatter::VisitFloat32Literal(Float32Literal* literal) {
  ostream_ << literal->data() << "f";
}

void LiteralFormatter::VisitFloat64Literal(Float64Literal* literal) {
  ostream_ << literal->data();
}

void LiteralFormatter::VisitFunction(Function* function) {
  ostream_ << *function;
}

void LiteralFormatter::VisitInt32Literal(Int32Literal* literal) {
  ostream_ << literal->data();
}

void LiteralFormatter::VisitInt64Literal(Int64Literal* literal) {
  ostream_ << literal->data() << "l";
}

void LiteralFormatter::VisitStringLiteral(StringLiteral* literal) {
  ostream_ << "\"";
  for (auto const ch : literal->data())
    ostream_ << AsPrintable(ch, '"');
  ostream_ << "\"";
}

//////////////////////////////////////////////////////////////////////
//
// ValueFormatter
//
class ValueFormatter final {
 public:
  explicit ValueFormatter(LiteralMap* literals, std::ostream* ostream);
  ~ValueFormatter() = default;

  std::ostream& Format(Value value);

 private:
  LiteralMap* literals_;
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(ValueFormatter);
};

ValueFormatter::ValueFormatter(LiteralMap* literals, std::ostream* ostream)
    : literals_(literals), ostream_(*ostream) {
}

std::ostream& ValueFormatter::Format(Value value) {
  if (value.kind == Value::Kind::Literal) {
    LiteralFormatter formatter(&ostream_);
    return formatter.Format(literals_->GetLiteral(value));
  }
  return ostream_ << value;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const BasicBlock& block) {
  return ostream << "block" << block.id();
}

std::ostream& operator<<(std::ostream& ostream, const Function& function) {
  return ostream << "function" << function.id();
}

std::ostream& operator<<(std::ostream& ostream,
                         const Instruction& instruction) {
  if (auto const block = instruction.basic_block())
    ostream << "bb:" << block->id();
  else
    ostream << "--:";
  ostream << instruction.id() << ":" << instruction.mnemonic();
  if (!instruction.outputs().empty()) {
    auto separator = "";
    for (auto const output : instruction.outputs()) {
      ostream << separator << output;
      separator = ", ";
    }
    ostream << " =";
  }
  if (auto const phi = instruction.as<PhiInstruction>()) {
    auto separator = " ";
    for (auto const phi_input : phi->phi_inputs()) {
      ostream << separator << *phi_input->basic_block() << " "
              << phi_input->value();
      separator = ", ";
    }
    return ostream;
  }
  auto separator = " ";
  for (auto const input : instruction.inputs()) {
    ostream << separator << input;
    separator = ", ";
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Literal& literal) {
  LiteralFormatter literal_formatter(&ostream);
  literal_formatter.Format(&literal);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, Opcode opcode) {
  return ostream << ToStringPiece(opcode);
}

std::ostream& operator<<(std::ostream& ostream,
                         const PrintableInstruction& printable) {
  auto const instruction = printable.instruction;
  auto const literals = printable.literals;
  ostream << instruction->mnemonic();

  // outputs
  if (!instruction->outputs().empty()) {
    auto separator = " ";
    for (auto output : instruction->outputs()) {
      ostream << separator << output;
      separator = ", ";
    }
    ostream << " =";
  }
  // inputs
  if (auto const phi = instruction->as<PhiInstruction>()) {
    auto separator = " ";
    for (auto const phi_input : phi->phi_inputs()) {
      ostream << separator << *phi_input->basic_block() << " "
              << PrintableValue(literals, phi_input->value());
      separator = ", ";
    }
    DCHECK_EQ(phi->outputs().size(), 1);
    return ostream;
  }
  auto separator = " ";
  if (!instruction->inputs().empty()) {
    for (auto const value : instruction->inputs()) {
      ostream << separator << PrintableValue(literals, value);
      separator = ", ";
    }
  }
  if (!instruction->block_operands().empty()) {
    for (auto const block : instruction->block_operands()) {
      ostream << separator << *block;
      separator = ", ";
    }
  }
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  return ostream << PrintableValue(value);
}

std::ostream& operator<<(std::ostream& ostream, const Value::Kind& kind) {
  static const char* const kinds[] = {
      "Invalid",
      "FloatRegister",
      "GeneralRegister",
      "Immediate",
      "Literal",
      "VirtualFloatRegister",
      "VirtualGeneralRegister",
      "NotUsed7",
      "Illegal",
  };
  return ostream
         << kinds[std::min(static_cast<size_t>(kind), arraysize(kinds) - 1)];
}

std::ostream& operator<<(std::ostream& ostream, const Value::Size& size) {
  static const char* const sizes[] = {
      "Size8", "Size16", "Size32", "Size64", "Illegal",
  };
  return ostream
         << sizes[std::min(static_cast<size_t>(size), arraysize(sizes) - 1)];
}

std::ostream& operator<<(std::ostream& ostream, const Value::Type& type) {
  static const char* const types[] = {
      "Integer", "Float", "Illegal",
  };
  return ostream
         << types[std::min(static_cast<size_t>(type), arraysize(types) - 1)];
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(LiteralMap* literals, std::ostream* ostream)
    : literals_(literals), ostream_(*ostream) {
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::FormatFunction(const Function* function) {
  ostream_ << *function << ":" << std::endl;
  for (auto const block : function->basic_blocks()) {
    ostream_ << *block << ":" << std::endl;

    ostream_ << "  // In: ";
    ostream_ << SortBasicBlocks(block->predecessors());
    ostream_ << std::endl;

    ostream_ << "  // Out: ";
    ostream_ << SortBasicBlocks(block->successors());
    ostream_ << std::endl;

    for (auto const phi_instruction : block->phi_instructions()) {
      ostream_ << "  ";
      FormatInstruction(phi_instruction);
      ostream_ << std::endl;
    }
    for (auto const instruction : block->instructions()) {
      ostream_ << "  ";
      FormatInstruction(instruction);
      ostream_ << std::endl;
    }
  }
}

std::ostream& TextFormatter::FormatInstruction(const Instruction* instruction) {
  return ostream_ << PrintableInstruction(literals_, instruction);
}

std::ostream& TextFormatter::FormatValue(Value value) {
  ValueFormatter formatter(literals_, &ostream_);
  return formatter.Format(value);
}

}  // namespace lir
}  // namespace elang
