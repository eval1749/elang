// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/semantics/editor.h"

#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"

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

void Editor::AddMember(Class* clazz, Token* name, Semantic* member) {
  DCHECK(!FindMember(clazz, name)) << *member;
  clazz->members_.insert(std::make_pair(name->atomic_string(), member));
}

void Editor::AddMethod(MethodGroup* method_group, Method* method) {
  DCHECK_EQ(method_group, method->method_group());
  DCHECK(std::find(method_group->methods_.begin(), method_group->methods_.end(),
                   method) == method_group->methods_.end())
      << method;
  method_group->methods_.push_back(method);
}

MethodGroup* Editor::EnsureMethodGroup(Class* clazz, Token* name) {
  auto const present = FindMember(clazz, name);
  if (auto const method_group = present->as<MethodGroup>())
    return method_group;
  auto const method_group = factory()->NewMethodGroup(clazz, name);
  if (!present)
    AddMember(clazz, name, method_group);
  return method_group;
}

Semantic* Editor::FindMember(Class* clazz, Token* name) const {
  auto const it = clazz->members_.find(name->atomic_string());
  return it == clazz->members_.end() ? nullptr : it->second;
}

void Editor::FixEnum(sm::Enum* enum_type,
                     const std::vector<EnumMember*>& members) {
  DCHECK(enum_type->members_.empty()) << *enum_type;
#ifndef NDEBUG
  for (auto const member : members)
    DCHECK_EQ(enum_type, member->owner());
#endif
  enum_type->members_.assign(members.begin(), members.end());
}

void Editor::SetSemanticOf(ast::Node* node, sm::Semantic* semantic) {
  DCHECK(node);
  DCHECK(semantic);
  auto const it = semantics()->semantic_map_.find(node);
  DCHECK(it == semantics()->semantic_map_.end())
      << *node << " old:" << *it->second << " new:" << *semantic;
  semantics()->semantic_map_[node] = semantic;
}

Semantic* Editor::SemanticOf(ast::Node* node) const {
  auto const semantic = TrySemanticOf(node);
  DCHECK(semantic) << *node;
  return semantic;
}

Semantic* Editor::TrySemanticOf(ast::Node* node) const {
  auto const it = semantics()->semantic_map_.find(node);
  return it == semantics()->semantic_map_.end() ? nullptr : it->second;
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
