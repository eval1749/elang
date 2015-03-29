// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/base/atomic_string.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace optimizer {

namespace {
//////////////////////////////////////////////////////////////////////
//
// NodePrinter
//
class NodePrinter final : public NodeVisitor {
 public:
  explicit NodePrinter(std::ostream& ostream)  // NOLINT
      : ostream_(ostream) {}

  ~NodePrinter() = default;

 private:
  void DoDefaultVisit(Node* node) final;
  void VisitBool(BoolNode* node) final;
  void VisitChar(CharNode* node) final;
  void VisitFloat32(Float32Node* node) final;
  void VisitFloat64(Float64Node* node) final;
  void VisitInt16(Int16Node* node) final;
  void VisitInt32(Int32Node* node) final;
  void VisitInt64(Int64Node* node) final;
  void VisitInt8(Int8Node* node) final;
  void VisitIntPtr(IntPtrNode* node) final;
  void VisitNull(NullNode* node) final;
  void VisitReference(ReferenceNode* node) final;
  void VisitString(StringNode* node) final;
  void VisitUInt16(UInt16Node* node) final;
  void VisitUInt32(UInt32Node* node) final;
  void VisitUInt64(UInt64Node* node) final;
  void VisitUInt8(UInt8Node* node) final;
  void VisitUIntPtr(UIntPtrNode* node) final;
  void VisitVoid(VoidNode* node) final;

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(NodePrinter);
};

struct AsInput {
  Node* node;
  explicit AsInput(Node* node) : node(node) {}
};

static char PrefixOf(const Node* node) {
  if (node->output_type()->is<TupleType>())
    return 't';
  if (node->IsControl())
    return 'c';
  if (node->IsEffect())
    return 'e';
  return 'r';
}

std::ostream& operator<<(std::ostream& ostream, const AsInput& input) {
  if (input.node->IsLiteral())
    return ostream << *input.node;
  return ostream << "%" << PrefixOf(input.node) << input.node->id();
}

void NodePrinter::DoDefaultVisit(Node* node) {
  DCHECK(!node->IsLiteral()) << *node;
  ostream_ << *node->output_type() << " " << AsInput(node) << " = "
           << node->mnemonic() << "(";
  auto separator = "";
  for (auto const input : node->inputs()) {
    ostream_ << separator << AsInput(input);
    separator = ", ";
  }
  if (auto const field_node = node->as<FieldInputNode>())
    ostream_ << separator << field_node->field();
  ostream_ << ")";
}

void NodePrinter::VisitBool(BoolNode* node) {
  ostream_ << std::boolalpha << node->data();
}

void NodePrinter::VisitChar(CharNode* node) {
  ostream_ << '\'' << AsPrintable(node->data(), '\'') << '\'';
}

void NodePrinter::VisitFloat32(Float32Node* node) {
  ostream_ << std::showpoint << node->data() << "f";
}

void NodePrinter::VisitFloat64(Float64Node* node) {
  ostream_ << std::showpoint << node->data();
}

void NodePrinter::VisitInt16(Int16Node* node) {
  ostream_ << "int16(" << node->data() << ")";
}

void NodePrinter::VisitInt32(Int32Node* node) {
  ostream_ << node->data();
}

void NodePrinter::VisitInt64(Int64Node* node) {
  ostream_ << node->data() << "l";
}

void NodePrinter::VisitInt8(Int8Node* node) {
  ostream_ << "int8(" << static_cast<int>(node->data()) << ")";
}

void NodePrinter::VisitIntPtr(IntPtrNode* node) {
  ostream_ << "intptr(" << node->data() << ")";
}

void NodePrinter::VisitNull(NullNode* node) {
  ostream_ << "null";
}

void NodePrinter::VisitReference(ReferenceNode* node) {
  ostream_ << *node->output_type() << " " << *node->name();
}

void NodePrinter::VisitString(StringNode* node) {
  ostream_ << '"';
  for (auto ch : node->data())
    ostream_ << AsPrintable(ch, '"');
  ostream_ << '"';
}

void NodePrinter::VisitUInt16(UInt16Node* node) {
  ostream_ << "uint16(" << node->data() << ")";
}

void NodePrinter::VisitUInt32(UInt32Node* node) {
  ostream_ << node->data() << "u";
}

void NodePrinter::VisitUInt64(UInt64Node* node) {
  ostream_ << node->data() << "ul";
}

void NodePrinter::VisitUInt8(UInt8Node* node) {
  ostream_ << "uint8(" << static_cast<int>(node->data()) << ")";
}

void NodePrinter::VisitUIntPtr(UIntPtrNode* node) {
  ostream_ << "uintptr(" << node->data() << ")";
}

void NodePrinter::VisitVoid(VoidNode* node) {
  ostream_ << "void";
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, FloatCondition condition) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, value) mnemonic,
      FOR_EACH_OPTIMIZER_FLOAT_CONDITION(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(condition);
  return ostream << (it < std::end(mnemonics) ? *it : "Invalid");
}

std::ostream& operator<<(std::ostream& ostream, IntCondition condition) {
  static const char* const mnemonics[] = {
#define V(Name, mnemonic, value) mnemonic,
      FOR_EACH_OPTIMIZER_INTEGER_CONDITION(V)
#undef V
  };
  auto const it = std::begin(mnemonics) + static_cast<size_t>(condition);
  return ostream << (it < std::end(mnemonics) ? *it : "Invalid");
}

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  NodePrinter printer(ostream);
  const_cast<Node&>(node).Accept(&printer);
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
