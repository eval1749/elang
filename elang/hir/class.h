// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_class_h)
#define INCLUDE_elang_hir_class_h

#include <vector>

#include "elang/hir/namespace.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  DECLARE_CASTABLE_CLASS(Class, Namespace);
  friend class Factory;

 public:
  const std::vector<Class*>& base_classes() const { return base_classes_; }

 private:
  Class(Namespace* outer, SimpleName* simple_name,
        const std::vector<Class*>& base_classes);
  ~Class() override;

  // NamespaceMember
  Namespace* ToNamespace() final;

  std::vector<Class*> base_classes_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace hir
}  // namespace elang

#endif  // !defined(INCLUDE_elang_hir_class_h)
