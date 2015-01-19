// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/compiler/ir/formatters/text_formatter.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/ir/visitor.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace compiler {
namespace ir {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public Visitor {
 public:
  explicit Formatter(std::ostream* ostream);
  ~Formatter() = default;

  void Format(const ir::Node* node);

 private:
// Visitor
#define V(Name) void Visit##Name(Name* node) final;
  FOR_EACH_CONCRETE_IR_NODE(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::Formatter(std::ostream* ostream) : ostream_(*ostream) {
}

void Formatter::Format(const Node* node) {
  const_cast<Node*>(node)->Accept(this);
}

// Visitor
void Formatter::VisitClass(Class* clazz) {
  // TODO(eval1749) We should output FQN for class.
  ostream_ << "(class " << clazz->ast_class()->name() << ")";
}

void Formatter::VisitEnum(Enum* enuz) {
  // TODO(eval1749) We should output FQN for enum.
  ostream_ << "(enum " << enuz->ast_enum()->name() << ")";
}

void Formatter::VisitLiteral(Literal* literal) {
  ostream_ << "(literal " << *literal->data() << ")";
}

void Formatter::VisitMethod(Method* method) {
  // TODO(eval1749) We should output FQN for method.
  ostream_ << "(method " << method->ast_method()->name() << " "
           << *method->signature() << ")";
}

void Formatter::VisitParameter(Parameter* parameter) {
  ostream_ << "(parameter " << *parameter->type() << ")";
}

void Formatter::VisitSignature(Signature* sig) {
  ostream_ << "(signature " << *sig->return_type() << " (";
  auto separator = "";
  for (auto const parameter : sig->parameters()) {
    ostream_ << separator << *parameter;
    separator = " ";
  }
  ostream_ << "))";
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  Formatter formatter(&ostream);
  formatter.Format(&node);
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

void TextFormatter::Format(const Node* node) {
  Formatter formatter(&ostream_);
  formatter.Format(node);
}

}  // namespace ir
}  // namespace compiler
}  // namespace elang
