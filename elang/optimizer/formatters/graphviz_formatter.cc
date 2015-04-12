// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iomanip>
#include <sstream>
#include <utility>
#include <vector>

#include "elang/optimizer/formatters/graphviz_formatter.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/base/atomic_string.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/opcode.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

namespace {

bool IsBasicBlockBegin(Node* node) {
  switch (node->opcode()) {
    case Opcode::Case:
    case Opcode::Entry:
    case Opcode::IfException:
    case Opcode::IfFalse:
    case Opcode::IfSuccess:
    case Opcode::IfTrue:
    case Opcode::Loop:
    case Opcode::Merge:
      return true;
  }
  return false;
}

bool IsBasicBlockEnd(Node* node) {
  switch (node->opcode()) {
    case Opcode::Exit:
    case Opcode::Jump:
    case Opcode::If:
    case Opcode::Ret:
    case Opcode::Throw:
      return true;
  }
  return false;
}

bool HasConstraint(Node* from, Node* to) {
  if (from->opcode() == Opcode::Loop)
    return false;
  return IsBasicBlockEnd(from) || IsBasicBlockBegin(from);
}

base::StringPiece PrefixOf(Node* node) {
  if (node->IsControl())
    return "%c";
  if (node->IsEffect())
    return "%e";
  if (node->IsTuple())
    return "%t";
  return "%r";
}

Node* ClusterOf(Node* node) {
  if (IsBasicBlockBegin(node))
    return node;
  if (auto const phi = node->as<PhiNode>())
    return phi->owner();
  if (auto const phi = node->as<EffectPhiNode>())
    return phi->owner();
  if (IsBasicBlockEnd(node) || node->opcode() == Opcode::Call ||
      node->opcode() == Opcode::GetData ||
      node->opcode() == Opcode::GetEffect ||
      node->opcode() == Opcode::GetTuple || node->opcode() == Opcode::Load ||
      node->opcode() == Opcode::Store || node->opcode() == Opcode::Parameter) {
    return ClusterOf(node->input(0));
  }
  return nullptr;
}

base::StringPiece NodeStyleOf(Node* node) {
  switch (node->opcode()) {
    case Opcode::Entry:
    case Opcode::Exit:
      return "style=diagonals";
  }
  if (node->IsControl())
    return "style=rounded";
  if (node->IsEffect())
    return "style=solid color=green";
  return "style=solid";
}

struct AsGraphvizLabel {
  Node* node;

  explicit AsGraphvizLabel(Node* node) : node(node) {}
};

struct AsGraphvizNode {
  Node* node;

  explicit AsGraphvizNode(Node* node) : node(node) {}
};

std::ostream& operator<<(std::ostream& ostream, const AsGraphvizLabel& wrapper);
std::ostream& operator<<(std::ostream& ostream, const AsGraphvizNode& wrapper);

//////////////////////////////////////////////////////////////////////
//
// EdgePrinter
//
class EdgePrinter final : public NodeVisitor {
 public:
  explicit EdgePrinter(std::ostream* ostream);
  ~EdgePrinter() = default;

 private:
  // NodeVisitor
  void DoDefaultVisit(Node* node) final;

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(EdgePrinter);
};

EdgePrinter::EdgePrinter(std::ostream* ostream) : ostream_(*ostream) {
}

void EdgePrinter::DoDefaultVisit(Node* node) {
  if (node->IsLiteral())
    return;
  if (auto const phi = node->as<PhiNode>()) {
    ostream_ << "  node" << node->id() << " -> node" << phi->owner()->id()
             << " [style=dashed]" << std::endl;
  } else if (auto const phi = node->as<EffectPhiNode>()) {
    ostream_ << "  node" << node->id() << " -> node" << phi->owner()->id()
             << " [style=dashed]" << std::endl;
  }

  auto index = 0;
  for (auto const input : node->inputs()) {
    if (input->IsLiteral()) {
      ++index;
      continue;
    }
    ostream_ << "  ";
    ostream_ << "node" << node->id() << ":i" << index << " -> "
             << "node" << input->id() << " ["
             << (node->opcode() == Opcode::Loop && index
                     ? " color=red constraint=true"
                     : "") << (input->IsControl() ? " style=bold" : "")
             << (input->IsData() ? " color=transparent" : "")
             << (node->IsEffect() ? " style=dotted constraint=true" : "")
             << (input->IsEffect() ? " style=dotted" : "") << "]" << std::endl;
    ++index;
  }
}

//////////////////////////////////////////////////////////////////////
//
// NodePrinter
//
class NodePrinter final : public NodeVisitor {
 public:
  explicit NodePrinter(std::ostream* ostream);
  ~NodePrinter() = default;

 private:
  // NodeVisitor
  void DoDefaultVisit(Node* node) final;

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(NodePrinter);
};

NodePrinter::NodePrinter(std::ostream* ostream) : ostream_(*ostream) {
}

void NodePrinter::DoDefaultVisit(Node* node) {
  if (node->IsLiteral())
    return;
  ostream_ << AsGraphvizNode(node) << std::endl;
}

std::ostream& operator<<(std::ostream& ostream,
                         const AsGraphvizLabel& wrapper) {
  static bool escape[256];
  if (!escape['<']) {
    escape['<'] = true;
    escape['>'] = true;
    escape['|'] = true;
    escape['\\'] = true;
    escape['"'] = true;
  }

  auto const node = wrapper.node;
  DCHECK(node->IsLiteral());
  std::stringstream stream;
  stream << *node;
  ostream << '"';
  for (auto const ch : stream.str()) {
    if (escape[ch & 0xFF])
      ostream << '\\';
    ostream << ch;
  }
  return ostream << '"';
}

std::ostream& operator<<(std::ostream& ostream, const AsGraphvizNode& wrapper) {
  auto const node = wrapper.node;
  auto const cluster = ClusterOf(node);
  ostream << "  ";
  if (cluster) {
    ostream << "subgraph cluster_" << cluster->id() << " {"
            << " style=filled;"
            << (cluster->opcode() == Opcode::Loop ? " color=\"#CCCCCC\""
                                                  : " color=\"#EEEEEE\"");
  }
  ostream << "node" << node->id() << " [shape=record " << NodeStyleOf(node)
          << " label=\"{{" << PrefixOf(node) << node->id() << "="
          << node->mnemonic();
  std::vector<std::pair<Node*, int>> literals;
  auto index = 0;
  for (auto const input : node->inputs()) {
    ostream << "|<i" << index << ">";
    if (input->IsLiteral())
      literals.push_back(std::make_pair(input, index));
    else
      ostream << PrefixOf(input) << input->id();
    ++index;
  }
  ostream << "}}\"];";

  for (auto const pair : literals) {
    auto const index = pair.second;
    auto const lit = pair.first;
    ostream << "lit" << node->id() << "_" << index
            << " [color=blue label=" << AsGraphvizLabel(lit) << "];"
            << "node" << node->id() << ":i" << index << " -> "
            << "lit" << node->id() << "_" << index
            << " [style=dashed color=blue]";
  }

  if (cluster)
    ostream << "}";

  return ostream;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const AsGraphviz& wrapper) {
  ostream << "digraph IR {" << std::endl
          // Node: when we set |concentrate| to true, "dot" is crashed with
          // "samples/statements/for.e".
          << "  concentrate=false" << std::endl
          << "  node [fontname=monospace fontsize=8, hieght=0.25]" << std::endl
          << "  overlap=false" << std::endl
          << "  rankdir=\"BT\"" << std::endl
          << "  ranksep=\"1.2 equally\"" << std::endl
          << "  splines=true" << std::endl
          << std::endl;

  auto const function = wrapper.function;
  DepthFirstTraversal<OnInputEdge, const Function> walker;

  ostream << "  // Nodes" << std::endl;
  NodePrinter node_printer(&ostream);
  walker.Traverse(function, &node_printer);

  ostream << std::endl;

  ostream << "  // Edges" << std::endl;
  EdgePrinter edge_printer(&ostream);
  walker.Traverse(function, &edge_printer);

  ostream << "}" << std::endl;
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
