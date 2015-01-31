// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/hir/formatters/text_formatter.h"

#include "base/strings/utf_string_conversions.h"
#include "elang/base/atomic_string.h"
#include "elang/hir/instructions.h"
#include "elang/hir/values.h"
#include "elang/hir/value_visitor.h"
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

// AsValue
struct AsValue {
  const Value* value;
  explicit AsValue(const Value& value) : value(&value) {}
};

const char* RegisterPrefixOf(const Instruction* instruction) {
  auto const type = instruction->output_type();
  if (type->is<BoolType>())
    return "%b";
  if (type->is<TupleType>())
    return "%t";
  if (type->register_class() == Type::RegisterClass::Float)
    return "%f";
  return "%r";
}

std::ostream& operator<<(std::ostream& ostream, const AsValue& thing) {
  auto const value = thing.value;
  if (auto const instruction = value->as<Instruction>()) {
    return ostream << RegisterPrefixOf(instruction) << instruction->id();
  }
  return ostream << *value;
}

// WithoutAddress
struct WithoutAddress {
  const Instruction* instruction;
  explicit WithoutAddress(const Instruction& instruction)
      : instruction(&instruction) {}
};

std::ostream& operator<<(std::ostream& ostream, const WithoutAddress& thing) {
  auto const instruction = thing.instruction;
  if (!instruction->type()->is<VoidType>()) {
    ostream << *instruction->output_type() << " " << AsValue(*instruction)
            << " = ";
  }
  ostream << instruction->opcode();

  if (auto const alloca = instruction->as<StackAllocInstruction>()) {
    ostream << " " << alloca->count();
    return ostream;
  }

  if (auto const get = instruction->as<GetInstruction>()) {
    ostream << " " << AsValue(*get->input(0)) << ", " << get->index();
    return ostream;
  }

  if (auto const phi = instruction->as<PhiInstruction>()) {
    auto separator = " ";
    for (auto const phi_input : phi->phi_inputs()) {
      ostream << separator << *phi_input->basic_block() << " "
              << AsValue(*phi_input->value());
      separator = ", ";
    }
    return ostream;
  }

  auto separator = " ";
  for (auto const input : instruction->inputs()) {
    ostream << separator << AsValue(*input);
    separator = ", ";
  }

  return ostream;
}

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
#define V(Name) void Visit##Name(Name* type) override;
  FOR_EACH_HIR_CONCRETE_TYPE(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TypeFormatter);
};

void TypeFormatter::Format(const Type* type) {
  const_cast<Type*>(type)->Accept(this);
}

void TypeFormatter::VisitExternalType(ExternalType* type) {
  ostream_ << type->name();
}

void TypeFormatter::VisitFunctionType(FunctionType* type) {
  ostream_ << *type->return_type() << "(" << *type->parameters_type() << ")";
}

void TypeFormatter::VisitPointerType(PointerType* type) {
  ostream_ << *type->pointee() << "*";
}

void TypeFormatter::VisitStringType(StringType* type) {
  DCHECK(type);
  ostream_ << "string";
}

void TypeFormatter::VisitTupleType(TupleType* type) {
  ostream_ << "{";
  auto separator = "";
  for (auto const member : type->members()) {
    ostream_ << separator << *member;
    separator = ", ";
  }
  ostream_ << "}";
}

#define V(Name, name, ...)                                  \
  void TypeFormatter::Visit##Name##Type(Name##Type* type) { \
    DCHECK(type);                                           \
    ostream_ << #name;                                      \
  }
FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// ValueFormatter
//
class ValueFormatter final : public ValueVisitor {
 public:
  explicit ValueFormatter(std::ostream& ostream) : ostream_(ostream) {}
  ~ValueFormatter() = default;

  void Format(const Value* type);

 private:
#define V(Name, ...) void Visit##Name(Name* type) final;
  FOR_EACH_HIR_VALUE(V)
#undef V
  void VisitInstruction(Instruction* value) final;

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(ValueFormatter);
};

void ValueFormatter::Format(const Value* type) {
  const_cast<Value*>(type)->Accept(this);
}

void ValueFormatter::VisitBasicBlock(BasicBlock* block) {
  ostream_ << "block" << block->id();
}

void ValueFormatter::VisitBoolLiteral(BoolLiteral* literal) {
  ostream_ << (literal->data() ? "true" : "false");
}

void ValueFormatter::VisitCharLiteral(CharLiteral* literal) {
  // TODO(eval1749) We should share this code among compiler, HIR, LIR, and
  // ASM.
  static char const xdigits[] = "0123456789ABCDEF";
  static char const escapes[] = "0------abtnvfr";
  auto const ch = literal->data();
  char buffer[7];
  if (ch <= 0x0D && escapes[ch] != '-') {
    buffer[0] = '\\';
    buffer[1] = escapes[ch];
    buffer[2] = 0;
  } else if (ch == '\'' || ch == '\\') {
    buffer[0] = '\\';
    buffer[1] = ch;
    buffer[2] = 0;
  } else if (ch < ' ' || ch >= 0x7F) {
    buffer[0] = '\\';
    buffer[1] = 'u';
    buffer[2] = xdigits[(ch >> 12) & 15];
    buffer[3] = xdigits[(ch >> 8) & 15];
    buffer[4] = xdigits[(ch >> 4) & 15];
    buffer[5] = xdigits[ch & 15];
    buffer[6] = 0;
  } else {
    buffer[0] = ch;
    buffer[1] = 0;
  }
  ostream_ << "'" << buffer << "'";
}

void ValueFormatter::VisitFloat32Literal(Float32Literal* literal) {
  ostream_ << literal->data() << "f";
}

void ValueFormatter::VisitFloat64Literal(Float64Literal* literal) {
  ostream_ << literal->data();
}

void ValueFormatter::VisitInt16Literal(Int16Literal* literal) {
  ostream_ << "int16(" << literal->data() << ")";
}

void ValueFormatter::VisitInt32Literal(Int32Literal* literal) {
  ostream_ << literal->data();
}

void ValueFormatter::VisitInt64Literal(Int64Literal* literal) {
  ostream_ << literal->data() << "l";
}

void ValueFormatter::VisitInt8Literal(Int8Literal* literal) {
  ostream_ << "int8(" << static_cast<int>(literal->data()) << ")";
}

void ValueFormatter::VisitFunction(Function* function) {
  ostream_ << "function" << function->id();
}

void ValueFormatter::VisitInstruction(Instruction* instruction) {
  ostream_ << *instruction;
}

void ValueFormatter::VisitReference(Reference* reference) {
  ostream_ << "`" << *reference->name() << "`";
}

void ValueFormatter::VisitNullLiteral(NullLiteral* literal) {
  ostream_ << "static_cast<" << *literal->type() << ">(null)";
}

void ValueFormatter::VisitStringLiteral(StringLiteral* literal) {
  // TODO(eval1749) We should share this code among compiler, HIR, LIR, and
  // ASM.
  static char const xdigits[] = "0123456789ABCDEF";
  static char const escapes[] = "0------abtnvfr";
  ostream_ << "\"";
  for (auto const ch : literal->data()) {
    char buffer[7];
    if (ch <= 0x0D && escapes[ch] != '-') {
      buffer[0] = '\\';
      buffer[1] = escapes[ch];
      buffer[2] = 0;
    } else if (ch == '"' || ch == '\\') {
      buffer[0] = '\\';
      buffer[1] = ch;
      buffer[2] = 0;
    } else if (ch < ' ' || ch >= 0x7F) {
      buffer[0] = '\\';
      buffer[1] = 'u';
      buffer[2] = xdigits[(ch >> 12) & 15];
      buffer[3] = xdigits[(ch >> 8) & 15];
      buffer[4] = xdigits[(ch >> 4) & 15];
      buffer[5] = xdigits[ch & 15];
      buffer[6] = 0;
    } else {
      buffer[0] = ch;
      buffer[1] = 0;
    }
    ostream_ << buffer;
  }
  ostream_ << "\"";
}

void ValueFormatter::VisitTupleLiteral(TupleLiteral* literal) {
  ostream_ << "{";
  auto index = 0;
  auto separator = "";
  for (auto const member : literal->type()->as<TupleType>()->members()) {
    DCHECK(member);
    ostream_ << separator << *literal->get(index);
    separator = ", ";
    ++index;
  }
  ostream_ << "}";
}

void ValueFormatter::VisitUInt16Literal(UInt16Literal* literal) {
  ostream_ << "uint16(" << literal->data() << ")";
}

void ValueFormatter::VisitUInt32Literal(UInt32Literal* literal) {
  ostream_ << literal->data() << "u";
}

void ValueFormatter::VisitUInt64Literal(UInt64Literal* literal) {
  ostream_ << literal->data() << "ul";
}

void ValueFormatter::VisitUInt8Literal(UInt8Literal* literal) {
  ostream_ << "uint8(" << static_cast<int>(literal->data()) << ")";
}

void ValueFormatter::VisitVoidValue(VoidValue* literal) {
  DCHECK(literal);
  ostream_ << "void";
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream,
                         const Instruction& instruction) {
  if (auto const basic_block = instruction.basic_block())
    ostream << "bb" << basic_block->id();
  else
    ostream << "--";
  ostream << ":" << instruction.id() << ":";
  return ostream << WithoutAddress(instruction);
}

std::ostream& operator<<(std::ostream& ostream, Opcode opcode) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, ...) mnemonic,
      FOR_EACH_HIR_INSTRUCTION(V)
#undef V
          "INVALID",
  };
  return ostream << mnemonics[std::min(static_cast<size_t>(opcode),
                                       arraysize(mnemonics) - 1)];
}

std::ostream& operator<<(std::ostream& ostream, const Thing& thing) {
  if (auto const value = thing.as<Value>())
    return ostream << *value;
  if (auto const type = thing.as<Type>())
    return ostream << *type;
  NOTREACHED();
  return ostream << "INVALID";
}

std::ostream& operator<<(std::ostream& ostream, const Type& type) {
  TypeFormatter type_formatter(ostream);
  type_formatter.Format(&type);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Value& value) {
  ValueFormatter value_formatter(ostream);
  value_formatter.Format(&value);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Value* value) {
  if (!value)
    return ostream << "(null)";
  return ostream << *value;
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
  ostream_ << *function << " " << *function->type() << std::endl;
  for (auto const block : function->basic_blocks()) {
    ostream_ << *block << ":" << std::endl;

    ostream_ << "  // In:";
    for (auto const predecessor : block->predecessors())
      ostream_ << " " << *predecessor;
    ostream_ << std::endl;

    ostream_ << "  // Out:";
    for (auto const successor : block->successors())
      ostream_ << " " << *successor;
    ostream_ << std::endl;

    for (auto const phi : block->phi_instructions())
      ostream_ << "  " << WithoutAddress(*phi) << std::endl;

    for (auto const instruction : block->instructions())
      ostream_ << "  " << WithoutAddress(*instruction) << std::endl;
  }
}

}  // namespace hir
}  // namespace elang
