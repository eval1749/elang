// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>
#include <vector>

#include "elang/compiler/semantics/formatters/text_formatter.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/visitor.h"
#include "elang/compiler/parameter_kind.h"

namespace base {
std::ostream& operator<<(std::ostream& ostream,
                         const base::StringPiece16& piece) {
  return ostream << base::UTF16ToUTF8(piece.as_string());
}
}  // namespace base

namespace elang {
namespace compiler {
namespace sm {

namespace {

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public Visitor {
 public:
  explicit Formatter(std::ostream* ostream);
  ~Formatter() = default;

  void Format(const sm::Semantic* semantic);

 private:
// Visitor
#define V(Name) void Visit##Name(Name* semantic) final;
  FOR_EACH_CONCRETE_SEMANTIC(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::Formatter(std::ostream* ostream) : ostream_(*ostream) {
}

void Formatter::Format(const Semantic* semantic) {
  const_cast<Semantic*>(semantic)->Accept(this);
}

// Visitor

// Element type of array type is omitting left most rank, e.g.
//  element_type_of(T[A]) = T
//  element_type_of(T[A][B}) = T[B]
//  element_type_of(T[A][B}[C]) = T[B][C]
void Formatter::VisitArrayType(ArrayType* semantic) {
  std::vector<ArrayType*> array_types;
  for (Type* runner = semantic; runner->is<ArrayType>();
       runner = runner->as<ArrayType>()->element_type()) {
    array_types.push_back(runner->as<ArrayType>());
  }
  ostream_ << *array_types.back()->element_type();
  for (auto array_type : array_types) {
    ostream_ << "[";
    auto separator = "";
    for (auto dimension : array_type->dimensions()) {
      ostream_ << separator;
      if (dimension >= 0)
        ostream_ << dimension;
      separator = ",";
    }
    ostream_ << "]";
  }
}

void Formatter::VisitClass(Class* semantic) {
  ostream_ << semantic->ast_class()->NewQualifiedName();
}

void Formatter::VisitEnum(Enum* semantic) {
  ostream_ << semantic->ast_enum()->NewQualifiedName();
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
  ostream_ << variable->storage() << " " << *variable->type() << " "
           << variable->ast_node()->name();
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Semantic& semantic) {
  Formatter formatter(&ostream);
  formatter.Format(&semantic);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, StorageClass storage_class) {
  static const char* const texts[] = {
#define V(Name) #Name,
      FOR_EACH_STORAGE_CLASS(V)
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

void TextFormatter::Format(const Semantic* semantic) {
  Formatter formatter(&ostream_);
  formatter.Format(semantic);
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
