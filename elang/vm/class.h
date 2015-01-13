// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_CLASS_H_
#define ELANG_VM_CLASS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/vm/namespace.h"

namespace elang {
namespace vm {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  DECLARE_VM_NODE_CLASS(Class, Namespace);

 public:
  const ZoneVector<Class*>& base_classes() const { return base_classes_; }

 private:
  Class(Zone* zone,
        Namespace* outer,
        AtomicString* simple_name,
        const std::vector<Class*>& base_classes);

  // NamespaceMember
  Namespace* ToNamespace() final;

  const ZoneVector<Class*> base_classes_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_CLASS_H_
