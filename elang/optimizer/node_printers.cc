// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "base/strings/utf_string_conversions.h"
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
  void VisitVoid(VoidNode* node) final;

#define V(Name, ...) void Visit##Name(Name##Node* node) final;
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V

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
  ostream_ << *node->output_type() << " " << AsInput(node) << " = "
           << node->mnemonic() << "(";
  auto separator = "";
  for (auto const input : node->inputs()) {
    ostream_ << separator << AsInput(input);
    separator = ", ";
  }
  ostream_ << ")";
}

void NodePrinter::VisitVoid(VoidNode* node) {
  ostream_ << "void";
}

#define V(Name, ...)                                                  \
  void NodePrinter::Visit##Name(Name##Node* literal) {                \
    ostream_ << literal->mnemonic() << "(" << literal->data() << ")"; \
  }
FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V
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
