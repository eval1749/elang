// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/lir/formatters/text_formatter.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/lir/instructions.h"
#include "elang/lir/literals.h"
#include "elang/lir/literal_map.h"
#include "elang/lir/literal_visitor.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace lir {
namespace isa {
base::StringPiece GetMnemonic(const Instruction* instruction);
}

namespace {

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
  switch (value.kind) {
    case Value::Kind::Invalid:
      return ostream_ << "invalid";
    case Value::Kind::GeneralRegister:
      return ostream_ << "%r" << value.data;
    case Value::Kind::FloatRegister:
      return ostream_ << "%f" << value.data;
    case Value::Kind::Immediate:
      return ostream_ << value.data;
    case Value::Kind::Literal: {
      LiteralFormatter formatter(&ostream_);
      return formatter.Format(literals_->GetLiteral(value));
    }
    case Value::Kind::VirtualRegister:
      return ostream_ << "%v" << value.data;
  }
  return ostream_ << "?Value" << static_cast<int>(value.kind);
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
    ostream << *block;
  else
    ostream << "orphan";
  ostream << "(" << instruction.id() << ")";
  return ostream << isa::GetMnemonic(&instruction);
}

std::ostream& operator<<(std::ostream& ostream, const Literal& literal) {
  LiteralFormatter literal_formatter(&ostream);
  literal_formatter.Format(&literal);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  switch (value.kind) {
    case Value::Kind::Invalid:
      return ostream << "invalid";
    case Value::Kind::GeneralRegister:
      return ostream << "%r" << value.data;
    case Value::Kind::FloatRegister:
      return ostream << "%f" << value.data;
    case Value::Kind::Immediate:
      return ostream << value.data;
    case Value::Kind::Literal:
      // Note: We can't print value of literal, since it is hold in
      // |LiteralMap|.
      return ostream << "#" << value.data;
    case Value::Kind::VirtualRegister:
      return ostream << "%v" << value.data;
  }
  return ostream << "?" << value.kind << "?" << value.data;
}

std::ostream& operator<<(std::ostream& ostream, const Value::Kind& kind) {
  static const char* const kinds[] = {
      "Invalid",
      "GeneralRegister",
      "FloatRegister",
      "Immediate",
      "Literal",
      "VirtualRegister",
      "NotUsed6",
      "NotUsed7",
      "Illegal",
  };
  return ostream
         << kinds[std::min(static_cast<size_t>(kind), arraysize(kinds) - 1)];
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
    for (auto const instruction : block->instructions()) {
      ostream_ << "  ";
      FormatInstruction(instruction);
      ostream_ << std::endl;
    }
  }
}

std::ostream& TextFormatter::FormatInstruction(const Instruction* instruction) {
  // outputs
  if (!instruction->outputs().empty()) {
    auto separator = "";
    for (auto const value : instruction->outputs()) {
      ostream_ << separator;
      FormatValue(value);
      separator = ", ";
    }
    ostream_ << " = ";
  }
  ostream_ << isa::GetMnemonic(instruction);
  // inputs
  if (!instruction->inputs().empty()) {
    auto separator = " ";
    for (auto const value : instruction->inputs()) {
      ostream_ << separator;
      FormatValue(value);
      separator = ", ";
    }
  }
  return ostream_;
}

std::ostream& TextFormatter::FormatValue(Value value) {
  ValueFormatter formatter(literals_, &ostream_);
  return formatter.Format(value);
}

}  // namespace lir
}  // namespace elang
