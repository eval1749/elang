// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/semantics/editor.h"

#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(CompilationSession* session) : CompilationSessionUser(session) {
}

Editor::~Editor() {
}

sm::Factory* Editor::factory() const {
  return session()->semantics_factory();
}

MethodGroup* Editor::EnsureMethodGroup(Class* clazz, Token* name) {
  auto const present = FindMember(clazz, name);
  if (auto const method_group = present->as<MethodGroup>())
    return method_group;
  return factory()->NewMethodGroup(clazz, name);
}

Semantic* Editor::FindMember(Semantic* container, Token* name) const {
  if (auto const clazz = container->as<Class>()) {
    auto const it = clazz->members_.find(name->atomic_string());
    return it == clazz->members_.end() ? nullptr : it->second;
  }
  if (auto const ns = container->as<Namespace>()) {
    auto const it = ns->members_.find(name->atomic_string());
    return it == ns->members_.end() ? nullptr : it->second;
  }
  NOTREACHED() << container << " " << name;
  return nullptr;
}

void Editor::FixEnumMember(sm::EnumMember* member, Value* value) {
  DCHECK(!member->value_) << *member << " " << value;
  member->value_ = value;
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
