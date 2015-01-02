// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/formatters/text_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/hir/instructions.h"
#include "elang/hir/operands.h"
#include "elang/hir/operand_visitor.h"
#include "elang/hir/types.h"
#include "elang/hir/type_visitor.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace hir {

namespace {

//////////////////////////////////////////////////////////////////////
//
// OperandFormatter
//
class OperandFormatter : public OperandVisitor {
 public:
  explicit OperandFormatter(std::ostream& ostream) : ostream_(ostream) {}
  ~OperandFormatter() = default;

  void Format(const Operand* type);

 private:
#define V(Name, ...) \
    void Visit##Name(Name* type) override;
  FOR_EACH_HIR_OPERAND(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(OperandFormatter);
};

void OperandFormatter::Format(const Operand* type) {
  const_cast<Operand*>(type)->Accept(this);
}

void OperandFormatter::VisitBasicBlock(BasicBlock* block) {
  ostream_ << *block;
}

void OperandFormatter::VisitFunction(Function* function) {
  ostream_ << *function;
}

void OperandFormatter::VisitNullLiteral(NullLiteral* literal) {
  ostream_ << *literal->type() << " null";
}

void OperandFormatter::VisitVoidLiteral(VoidLiteral* literal) {
  DCHECK(literal);
  ostream_ << "void";
}

#define V(Name, name, ...) \
  void OperandFormatter::Visit##Name##Literal(Name##Literal* literal) { \
    ostream_ << *literal->type() << " " << \
        static_cast<Literal*>(literal)->name##_value(); \
  }
FOR_EACH_HIR_LITERAL_OPERAND(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// TypeFormatter
//
class TypeFormatter : public TypeVisitor {
 public:
  explicit TypeFormatter(std::ostream& ostream) : ostream_(ostream) {}
  ~TypeFormatter() = default;

  void Format(const Type* type);

 private:
#define V(Name, ...) \
  void Visit##Name##Type(Name##Type* type) override;
  FOR_EACH_HIR_TYPE(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TypeFormatter);
};

void TypeFormatter::Format(const Type* type) {
  const_cast<Type*>(type)->Accept(this);
}

void TypeFormatter::VisitFunctionType(FunctionType* type) {
  ostream_ << *type->return_type() << "(" << *type->parameters_type() << ")";
}

void TypeFormatter::VisitStringType(StringType* type) {
  DCHECK(type);
  ostream_ << "string";
}

#define V(Name, name, ...) \
  void TypeFormatter::Visit##Name##Type(Name##Type* type) { \
    DCHECK(type); \
    ostream_ << #name; \
  }
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const BasicBlock& block) {
  return ostream << "block" << block.id();
}

std::ostream& operator<<(std::ostream& ostream, const Function& function) {
  return ostream << "function_" << &function;
}

std::ostream& operator<<(std::ostream& ostream, const Operand& operand) {
  OperandFormatter operand_formatter(ostream);
  operand_formatter.Format(&operand);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Type& type) {
  TypeFormatter type_formatter(ostream);
  type_formatter.Format(&type);
  return ostream;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(std::ostream* ostream) : ostream_(*ostream) {
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::FormatFunction(const Function* function) {
  ostream_ << "Function " << *function->type() << std::endl;
  auto first_block = true;
  for (auto const block : function->basic_blocks()) {
    if (first_block)
      first_block = false;
    else
      ostream_ << std::endl;
    ostream_ << *block << ":" << std::endl;
    ostream_ << "  // In:";
    for (auto const user : block->users()) {
      ostream_ << " " << *user->instruction()->basic_block();
    }
    ostream_ << std::endl;
    ostream_ << "  // Out:";
    {
      auto const last = block->last_instruction();
      auto const num_operands = last->CountOperands();
      for (auto nth = 0; nth < num_operands; ++nth) {
        if (auto const target = last->OperandAt(nth)->as<BasicBlock>())
          ostream_ << " " << *target;
      }
    }
    ostream_ << std::endl;
    for (auto const instruction : block->instructions()) {
      ostream_ << "  ";
      FormatInstruction(instruction);
      ostream_ << std::endl;
    }
  }
}

std::ostream& TextFormatter::FormatInstruction(const Instruction* instruction) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, ...) mnemonic,
    FOR_EACH_HIR_INSTRUCTION(V)
#undef V
  };

  if (!instruction->type()->is<VoidType>())
    ostream_ << *instruction->output_type() << "%r = " << instruction->id();
  ostream_ << mnemonics[instruction->opcode()];
  const char* separator = " ";
  auto const num_operands = instruction->CountOperands();
  for (auto nth = 0; nth < num_operands; ++nth) {
    auto const operand = instruction->OperandAt(nth);
    ostream_ << separator << *operand;
    separator = ", ";
  }

  return ostream_;
}

}  // namespace hir
}  // namespace elang
