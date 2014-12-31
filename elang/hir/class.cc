// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/class.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Namespace* outer,
             AtomicString* simple_name,
             const std::vector<Class*>& base_classes)
    : Namespace(outer, simple_name), base_classes_(base_classes) {
  // TODO(eval1749) NYI default base class |Object|.
  // TODO(eval1749) NYI validate |base_classes|, |base_classes[0]| must be
  // a class rather than an interface.
}

Class::~Class() {
}

// NamespaceMember
Namespace* Class::ToNamespace() {
  return nullptr;
}

}  // namespace hir
}  // namespace elang
