// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/cg/type_mapper.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/hir/factory.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// TypeMapper
//
TypeMapper::TypeMapper(CompilationSession* session, hir::Factory* factory)
    : CompilationSessionUser(session), factory_(factory) {
#define V(Name, name, ...)                                     \
  InstallType(session->PredefinedTypeOf(PredefinedName::Name), \
              types()->name##_type());
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
}

TypeMapper::~TypeMapper() {
}

hir::TypeFactory* TypeMapper::types() const {
  return factory()->types();
}

void TypeMapper::InstallType(sm::Type* type, hir::Type* hir_type) {
  DCHECK(!type_map_.count(type));
  type_map_[type] = hir_type;
}

hir::Type* TypeMapper::Map(sm::Type* type) {
  auto const it = type_map_.find(type);
  if (it != type_map_.end())
    return it->second;

  if (auto const array_type = type->as<sm::ArrayType>()) {
    std::vector<int> dimensions(array_type->dimensions().begin(),
                                array_type->dimensions().end());
    auto const hir_array_type =
        types()->NewArrayType(Map(array_type->element_type()), dimensions);
    auto const hir_type = types()->NewPointerType(hir_array_type);
    InstallType(type, hir_type);
    return hir_type;
  }

  if (auto const clazz = type->as<sm::Class>()) {
    // sm::Class => hir::ExternalType(class_name)
    auto const hir_type =
        types()->NewExternalType(session()->QualifiedNameOf(clazz));
    InstallType(type, hir_type);
    return hir_type;
  }

  if (auto const signature = type->as<sm::Signature>()) {
    // sm::Signature => hir::FunctionType(return_type, parameter_types)
    auto const arity = signature->maximum_arity();
    if (!arity) {
      auto const hir_type = types()->NewFunctionType(
          Map(signature->return_type()), types()->void_type());
      InstallType(type, hir_type);
      return hir_type;
    }
    if (arity == 1) {
      auto const hir_type =
          types()->NewFunctionType(Map(signature->return_type()),
                                   Map(signature->parameters()[0]->type()));
      InstallType(type, hir_type);
      return hir_type;
    }
    std::vector<hir::Type*> members(arity);
    members.resize(0);
    for (auto const parameter : signature->parameters())
      members.push_back(Map(parameter->type()));
    auto const tuple = types()->NewTupleType(members);
    auto const hir_type =
        types()->NewFunctionType(Map(signature->return_type()), tuple);
    InstallType(type, hir_type);
    return hir_type;
  }

  NOTREACHED() << *type;
  return nullptr;
}

hir::Type* TypeMapper::Map(PredefinedName name) {
  return Map(session()->PredefinedTypeOf(name));
}

}  // namespace compiler
}  // namespace elang
