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
#include "elang/compiler/parameter_kind.h"

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
void Formatter::VisitArrayType(ArrayType* node) {
  ostream_ << *node->element_type() << "[";
  auto separator = "";
  for (auto dimension : node->dimensions()) {
    ostream_ << separator;
    if (dimension >= 0)
      ostream_ << dimension;
    separator = ",";
  }
  ostream_ << "]";
}

void Formatter::VisitClass(Class* node) {
  ostream_ << node->ast_class()->NewQualifiedName();
}

void Formatter::VisitEnum(Enum* node) {
  ostream_ << node->ast_enum()->NewQualifiedName();
}

void Formatter::VisitLiteral(Literal* literal) {
  ostream_ << *literal->data();
}

void Formatter::VisitMethod(Method* method) {
  // TODO(eval1749) We should output FQN for method.
  ostream_ << *method->return_type() << " "
           << method->ast_method()->NewQualifiedName() << "(";
  auto separator = "";
  for (auto const parameter : method->parameters()) {
    ostream_ << separator << *parameter;
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitParameter(Parameter* parameter) {
  ostream_ << *parameter->type();
  if (parameter->kind() == ParameterKind::Rest)
    ostream_ << "...";
  ostream_ << " " << *parameter->name();
  if (parameter->kind() == ParameterKind::Optional)
    ostream_ << " = " << *parameter->default_value();
}

void Formatter::VisitSignature(Signature* signature) {
  ostream_ << *signature->return_type() << " ";
  auto separator = "";
  for (auto const parameter : signature->parameters()) {
    ostream_ << separator << *parameter;
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitVariable(Variable* variable) {
  ostream_ << variable->ast_node()->name() << "@" << variable->storage();
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  Formatter formatter(&ostream);
  formatter.Format(&node);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, StorageClass storage_class) {
  static const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_IR_STORAGE_CLASS(V)
#undef V
          "Invalid",
  };
  return ostream << texts[std::min(static_cast<size_t>(storage_class),
                                   arraysize(texts) - 1)];
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
