// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/cg/type_mapper.h"

#include "base/logging.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/hir/factory.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// TypeMapper
//
TypeMapper::TypeMapper(hir::Factory* factory, NameResolver* name_resolver)
    : factory_(factory), name_resolver_(name_resolver) {
#define V(Name, ...)                                                  \
  InstallType(name_resolver->GetPredefinedType(PredefinedName::Name), \
              factory->types()->Get##Name##Type());
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
}

TypeMapper::~TypeMapper() {
}

CompilationSession* TypeMapper::session() {
  return name_resolver()->session();
}

hir::Type* TypeMapper::Map(ir::Type* type) {
  auto const it = type_map_.find(type);
  if (it != type_map_.end())
    return it->second;
  if (auto const clazz = type->as<ir::Class>()) {
    auto const hir_type = factory()->types()->NewExternalType(
        session()->NewAtomicString(clazz->ast_class()->NewQualifiedName()));
    InstallType(type, hir_type);
    return hir_type;
  }
  NOTREACHED() << *type;
  return nullptr;
}

void TypeMapper::InstallType(ir::Type* type, hir::Type* hir_type) {
  DCHECK(!type_map_.count(type));
  type_map_[type] = hir_type;
}

}  // namespace compiler
}  // namespace elang
