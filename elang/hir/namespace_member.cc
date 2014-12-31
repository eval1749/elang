// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/namespace_member.h"

#include "elang/hir/namespace.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
NamespaceMember::NamespaceMember(Namespace* outer, AtomicString* simple_name)
    : outer_(outer), simple_name_(simple_name) {
}

NamespaceMember::~NamespaceMember() {
}

Namespace* NamespaceMember::ToNamespace() {
  return nullptr;
}

bool NamespaceMember::IsDescendantOf(const NamespaceMember* other) const {
  for (auto runner = outer_; runner; runner = runner->outer_) {
    if (runner == other)
      return true;
  }
  return false;
}

}  // namespace hir
}  // namespace elang
