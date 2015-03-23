// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_visitor.h"

namespace elang {
namespace optimizer {

namespace {
//////////////////////////////////////////////////////////////////////
//
// TypePrinter
//
class TypePrinter final : public TypeVisitor {
 public:
  explicit TypePrinter(std::ostream* ostream) : ostream_(*ostream) {}
  ~TypePrinter() final = default;

 private:
#define V(Name) void Visit##Name(Name* type) final;
  FOR_EACH_OPTIMIZER_CONCRETE_TYPE(V)
#undef V

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TypePrinter);
};

void TypePrinter::VisitArrayType(ArrayType* type) {
  ostream_ << *type->element_type();
  ostream_ << "[";
  auto separator = "";
  for (auto dimension : type->dimensions()) {
    ostream_ << separator;
    if (dimension >= 0)
      ostream_ << dimension;
    separator = ", ";
  }
  ostream_ << "]";
}

void TypePrinter::VisitControlType(ControlType* type) {
  ostream_ << "control";
}

void TypePrinter::VisitEffectType(EffectType* type) {
  ostream_ << "effect";
}

void TypePrinter::VisitExternalType(ExternalType* type) {
  ostream_ << *type->name();
}

void TypePrinter::VisitFunctionType(FunctionType* type) {
  ostream_ << *type->return_type() << "(";
  if (auto const tuple = type->parameters_type()->as<TupleType>()) {
    auto separator = "";
    for (auto const component : tuple->components()) {
      ostream_ << separator << *component;
      separator = ", ";
    }
  } else {
    ostream_ << *type->parameters_type();
  }
  ostream_ << ")";
}

void TypePrinter::VisitPointerType(PointerType* type) {
  ostream_ << *type->pointee() << "*";
}

void TypePrinter::VisitStringType(StringType* type) {
  DCHECK(type);
  ostream_ << "string";
}

void TypePrinter::VisitTupleType(TupleType* type) {
  ostream_ << "{";
  auto separator = "";
  for (auto const component : type->components()) {
    ostream_ << separator << *component;
    separator = ", ";
  }
  ostream_ << "}";
}

#define V(Name, name, ...)                                \
  void TypePrinter::Visit##Name##Type(Name##Type* type) { \
    DCHECK(type);                                         \
    ostream_ << #name;                                    \
  }
FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V
}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Type& type) {
  TypePrinter printer(&ostream);
  const_cast<Type&>(type).Accept(&printer);
  return ostream;
}

}  // namespace optimizer
}  // namespace elang
