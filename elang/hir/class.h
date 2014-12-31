// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_CLASS_H_
#define ELANG_HIR_CLASS_H_

#include <vector>

#include "elang/base/zone_vector.h"
#include "elang/hir/namespace.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  DECLARE_HIR_NODE_CLASS(Class, Namespace);

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

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_CLASS_H_
