// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analyze/predefined_types.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token_data.h"

namespace elang {
namespace compiler {

namespace {

ast::Class* FindClass(CompilationSession* session, PredefinedName type) {
  auto const name = session->name_for(type);
  auto const member = session->system_namespace()->FindMember(name);
  if (!member) {
    session->AddError(ErrorCode::PredefinedNamesNameNotFound,
                      session->NewToken(SourceCodeRange(), name));
    return nullptr;
  }
  if (auto const clazz = member->as<ast::Class>())
    return clazz;
  session->AddError(ErrorCode::PredefinedNamesNameNotClass,
                    session->NewToken(SourceCodeRange(), name));
  return nullptr;
}

}  // namespace

PredefinedTypes::PredefinedTypes(CompilationSession* session) {
#define V(Name)                                       \
  types_[static_cast<size_t>(PredefinedName::Name)] = \
      FindClass(session, PredefinedName::Name);
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
}

PredefinedTypes::~PredefinedTypes() {
}

ast::Class* PredefinedTypes::type_from(PredefinedName name) const {
  return types_[static_cast<size_t>(name)];
}

}  // namespace compiler
}  // namespace elang
