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

namespace {
void ComputeBaseClasses(Class* clazz, ZoneUnorderedSet<Class*>* classes) {
  if (classes->count(clazz))
    return;
  classes->insert(clazz);
  for (auto const base_class : clazz->direct_base_classes())
    ComputeBaseClasses(base_class, classes);
}
}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Editor
//
Editor::Editor(CompilationSession* session) : CompilationSessionUser(session) {
}

Editor::~Editor() {
}

sm::Factory* Editor::factory() const {
  return session()->semantic_factory();
}

void Editor::FixClassBase(Class* clazz,
                          const std::vector<Class*>& direct_base_classes) {
  DCHECK(!clazz->has_base()) << clazz;
  clazz->direct_base_classes_.assign(direct_base_classes.begin(),
                                     direct_base_classes.end());
  for (auto const base_class : direct_base_classes)
    ComputeBaseClasses(base_class, &clazz->base_classes_);
  clazz->has_base_ = true;
}

void Editor::FixEnumBase(Enum* enum_type, Type* enum_base) {
  DCHECK(!enum_type->enum_base_) << enum_type;
  enum_type->enum_base_ = enum_base;
}

void Editor::FixEnumMember(EnumMember* member, Value* value) {
  DCHECK(value);
  DCHECK(member->owner()->has_base()) << member;
  DCHECK(!member->value_) << member;
  member->value_ = value;
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
