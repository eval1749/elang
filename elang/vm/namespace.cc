// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/namespace.h"

#include "base/logging.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
Namespace::Namespace(Zone* zone, Namespace* outer, AtomicString* name)
    : NamespaceMember(outer, name), map_(zone) {
}

void Namespace::AddMember(NamespaceMember* member) {
  DCHECK(!FindMember(member->name()));
  map_[member->name()] = member;
}

NamespaceMember* Namespace::FindMember(AtomicString* name) {
  auto const it = map_.find(name);
  return it == map_.end() ? nullptr : it->second;
}

// NamespaceMember
Namespace* Namespace::ToNamespace() {
  return this;
}

}  // namespace vm
}  // namespace elang
