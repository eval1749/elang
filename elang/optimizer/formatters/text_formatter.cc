// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iomanip>

#include "elang/optimizer/formatters/text_formatter.h"

#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/as_printable.h"
#include "elang/base/atomic_string.h"
#include "elang/optimizer/depth_first_traversal.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_visitor.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace optimizer {

namespace {
//////////////////////////////////////////////////////////////////////
//
// Printer
//
class Printer final : public NodeVisitor {
 public:
  explicit Printer(std::ostream* ostream);
  ~Printer() = default;

 private:
  // NodeVisitor
  void DoDefaultVisit(Node* node) final;

  int counter_;
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(Printer);
};

Printer::Printer(std::ostream* ostream) : counter_(0), ostream_(*ostream) {
}

void Printer::DoDefaultVisit(Node* node) {
  if (node->IsLiteral())
    return;
  ostream_ << base::StringPrintf("%04d: ", counter_) << *node << std::endl;
  ++counter_;
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream,
                         const AsReversePostOrder& thing) {
  auto const function = thing.function;
  ostream << function << std::endl;
  Printer printer(&ostream);
  DepthFirstTraversal<OnInputEdge, const Function> walker;
  walker.Traverse(thing.function, &printer);
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
