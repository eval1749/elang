// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/cg/type_mapper.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
#include "elang/hir/factory.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace compiler {

namespace {
ir::Type* ValueOfPredefinedType(CompilationSession* session,
                                PredefinedName name) {
  auto const ast_class = session->GetPredefinedType(name);
  DCHECK(ast_class) << "Not in System namespace " << name;
  auto const ir_class = session->semantics()->ValueOf(ast_class);
  DCHECK(ir_class) << "Not resolved " << name;
  DCHECK(ir_class->is<ir::Class>());
  return ir_class->as<ir::Class>();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// TypeMapper
//
TypeMapper::TypeMapper(CompilationSession* session, hir::Factory* factory)
    : CompilationSessionUser(session), factory_(factory) {
#define V(Name, name, ...)                                          \
  InstallType(ValueOfPredefinedType(session, PredefinedName::Name), \
              types()->name##_type());
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
}

TypeMapper::~TypeMapper() {
}

hir::TypeFactory* TypeMapper::types() const {
  return factory()->types();
}

void TypeMapper::InstallType(ir::Type* type, hir::Type* hir_type) {
  DCHECK(!type_map_.count(type));
  type_map_[type] = hir_type;
}

hir::Type* TypeMapper::Map(ir::Type* type) {
  auto const it = type_map_.find(type);
  if (it != type_map_.end())
    return it->second;

  if (auto const clazz = type->as<ir::Class>()) {
    // ir::Class => hir::ExternalType(class_name)
    auto const hir_type = types()->NewExternalType(
        session()->NewAtomicString(clazz->ast_class()->NewQualifiedName()));
    InstallType(type, hir_type);
    return hir_type;
  }

  if (auto const signature = type->as<ir::Signature>()) {
    // ir::Signature => hir::FunctionType(return_type, parameter_types)
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
  return Map(ValueOfPredefinedType(session(), name));
}

}  // namespace compiler
}  // namespace elang
