// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/vm/class.h"

namespace elang {
namespace vm {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Zone* zone,
             Namespace* outer,
             AtomicString* simple_name,
             const std::vector<Class*>& base_classes)
    : Namespace(zone, outer, simple_name), base_classes_(zone, base_classes) {
  // TODO(eval1749) NYI default base class |Object|.
  // TODO(eval1749) NYI validate |base_classes|, |base_classes[0]| must be
  // a class rather than an interface.
}

// NamespaceMember
Namespace* Class::ToNamespace() {
  return nullptr;
}

}  // namespace vm
}  // namespace elang
