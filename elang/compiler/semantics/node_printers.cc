// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <ostream>
#include <string>
#include <vector>

#include "base/containers/adapters.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/visitor.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/token.h"

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

struct AsPath {
  Semantic* last_component;
};

std::ostream& operator<<(std::ostream& ostream, const AsPath& path) {
  std::vector<Semantic*> components;
  for (auto runner = path.last_component; runner && runner->name();
       runner = runner->outer()) {
    components.push_back(runner);
  }
  auto separator = "";
  for (auto component : base::Reversed(components)) {
    ostream << separator << component->name();
    separator = ".";
  }
  return ostream;
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
    auto separator2 = "";
    for (auto dimension : array_type->dimensions()) {
      ostream_ << separator;
      if (dimension >= 0)
        ostream_ << separator2 << dimension;
      separator = ",";
      separator2 = " ";
    }
    ostream_ << "]";
  }
}

void Formatter::VisitClass(Class* node) {
  if (!node->has_base()) {
    ostream_ << "#" << AsPath{node};
    return;
  }
  ostream_ << AsPath{node};
}

void Formatter::VisitConst(Const* node) {
  if (node->has_value()) {
    ostream_ << "const " << node->type() << " " << AsPath{node} << " = "
             << node->value();
    return;
  }
  ostream_ << "const ? " << AsPath{node} << " = ?";
}

void Formatter::VisitEnum(Enum* node) {
  if (!node->has_base()) {
    ostream_ << "#enum " << AsPath{node};
    return;
  }
  ostream_ << "enum " << AsPath{node} << " : " << AsPath{node->enum_base()}
           << " {";
  auto separator = "";
  for (auto const member : node->members()) {
    ostream_ << separator << *member->name();
    if (member->has_value())
      ostream_ << " = " << *member->value();
    separator = ", ";
  }
  ostream_ << "}";
}

void Formatter::VisitEnumMember(EnumMember* node) {
  ostream_ << AsPath{node};
  if (!node->has_value())
    return;
  ostream_ << " = " << *node->value();
}

void Formatter::VisitInvalidValue(InvalidValue* node) {
  ostream_ << "InvalidValue(" << *node->type() << ", " << node->token() << ")";
}

void Formatter::VisitField(Field* node) {
  if (node->has_type())
    ostream_ << node->type();
  else
    ostream_ << '?';
  ostream_ << ' ' << AsPath{node};
}

void Formatter::VisitLiteral(Literal* literal) {
  ostream_ << *literal->data();
}

void Formatter::VisitMethod(Method* method) {
  ostream_ << *method->return_type() << " " << AsPath{method} << "(";
  auto separator = "";
  for (auto const parameter : method->parameters()) {
    ostream_ << separator << *parameter->type();
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitMethodGroup(MethodGroup* method_group) {
  ostream_ << *method_group->owner() << "." << method_group->name() << "{";
  auto separator = "";
  for (auto const method : method_group->methods()) {
    ostream_ << separator << *method;
    separator = ", ";
  }
  ostream_ << "}";
}

void Formatter::VisitNamespace(Namespace* node) {
  if (!node->name()) {
    ostream_ << "global_namespace";
    return;
  }
  ostream_ << "namespace " << AsPath{node};
}

void Formatter::VisitParameter(Parameter* parameter) {
  ostream_ << *parameter->type();
  if (parameter->kind() == ParameterKind::Rest)
    ostream_ << "...";
  ostream_ << " " << *parameter->name();
  if (parameter->kind() == ParameterKind::Optional)
    ostream_ << " = " << *parameter->default_value();
}

void Formatter::VisitPointerType(PointerType* node) {
  ostream_ << node->pointee() << "*";
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

void Formatter::VisitUndefinedType(UndefinedType* node) {
  ostream_ << "UndefinedType(" << *node->token() << ")";
}

void Formatter::VisitVariable(Variable* variable) {
  ostream_ << variable->storage() << " " << *variable->type() << " "
           << variable->name();
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Semantic& semantic) {
  Formatter(&ostream).Format(&semantic);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Semantic* semantic) {
  if (!semantic)
    return ostream << "nil";
  return ostream << *semantic;
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

}  // namespace sm
}  // namespace compiler
}  // namespace elang
