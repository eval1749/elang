// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/namespace_member.h"

#include "elang/vm/namespace.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
NamespaceMember::NamespaceMember(Namespace* outer, AtomicString* name)
    : outer_(outer), name_(name) {
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

}  // namespace vm
}  // namespace elang
