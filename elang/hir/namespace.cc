// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/namespace.h"

#include "base/logging.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Namespace* outer, AtomicString* simple_name)
    : NamespaceMember(outer, simple_name) {
}

Namespace::~Namespace() {
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK(!FindMember(member->simple_name()));
  map_[member->simple_name()] = member;
}

NamespaceMember* Namespace::FindMember(AtomicString* simple_name) {
  auto const it = map_.find(simple_name);
  return it == map_.end() ? nullptr : it->second;
}

// NamespaceMember
Namespace* Namespace::ToNamespace() {
  return this;
}

}  // namespace hir
}  // namespace elang
