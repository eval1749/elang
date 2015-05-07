// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/translate/type_mapper.h"

#include "base/logging.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

namespace {
sm::Type* ValueOfPredefinedType(cm::CompilationSession* session,
                                cm::PredefinedName name) {
  auto const ast_class = session->PredefinedTypeOf(name);
  DCHECK(ast_class) << "Not in System namespace " << name;
  auto const sm_class = session->semantics()->SemanticOf(ast_class);
  DCHECK(sm_class) << "Not resolved " << name;
  DCHECK(sm_class->is<sm::Class>());
  return sm_class->as<sm::Class>();
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// IrTypeMapper
//
IrTypeMapper::IrTypeMapper(cm::CompilationSession* session,
                           ir::TypeFactory* type_factory)
    : cm::CompilationSessionUser(session), type_factory_(type_factory) {
#define V(Name, name, ...)                                              \
  InstallType(ValueOfPredefinedType(session, cm::PredefinedName::Name), \
              type_factory_->name##_type());
  FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V
}

IrTypeMapper::~IrTypeMapper() {
}

void IrTypeMapper::InstallType(sm::Type* sm_type, ir::Type* ir_type) {
  DCHECK(!sm_type_map_.count(sm_type));
  sm_type_map_[sm_type] = ir_type;
}

ir::Type* IrTypeMapper::Map(sm::Type* sm_type) {
  auto const it = sm_type_map_.find(sm_type);
  if (it != sm_type_map_.end())
    return it->second;

  if (auto const sm_array_type = sm_type->as<sm::ArrayType>()) {
    std::vector<int> dimensions(sm_array_type->dimensions().begin(),
                                sm_array_type->dimensions().end());
    auto const ir_array_type = type_factory_->NewArrayType(
        Map(sm_array_type->element_type()), dimensions);
    auto const ir_type = type_factory_->NewPointerType(ir_array_type);
    InstallType(sm_type, ir_type);
    return ir_type;
  }

  if (auto const sm_class = sm_type->as<sm::Class>()) {
    // sm::Class => ir::ExternalType(class_name)
    auto const ir_type = type_factory_->NewExternalType(
        session()->NewAtomicString(sm_class->ast_class()->NewQualifiedName()));
    InstallType(sm_type, ir_type);
    return ir_type;
  }

  if (auto const signature = sm_type->as<sm::Signature>()) {
    // sm::Signature => ir::FunctionType(return_type, parameter_types)
    auto const arity = signature->maximum_arity();
    if (!arity) {
      auto const ir_type = type_factory_->NewFunctionType(
          Map(signature->return_type()), type_factory_->void_type());
      InstallType(sm_type, ir_type);
      return ir_type;
    }
    if (arity == 1) {
      auto const ir_type = type_factory_->NewFunctionType(
          Map(signature->return_type()),
          Map(signature->parameters()[0]->type()));
      InstallType(sm_type, ir_type);
      return ir_type;
    }
    std::vector<ir::Type*> members(arity);
    members.resize(0);
    for (auto const parameter : signature->parameters())
      members.push_back(Map(parameter->type()));
    auto const tuple = type_factory_->NewTupleType(members);
    auto const ir_type =
        type_factory_->NewFunctionType(Map(signature->return_type()), tuple);
    InstallType(sm_type, ir_type);
    return ir_type;
  }

  NOTREACHED() << *sm_type;
  return nullptr;
}

ir::Type* IrTypeMapper::Map(cm::PredefinedName name) {
  return Map(ValueOfPredefinedType(session(), name));
}

}  // namespace compiler
}  // namespace elang
