// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_class_h)
#define INCLUDE_elang_hir_class_h

#include "elang/hir/namespace.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Class, NamespaceMember);
  friend class Factory;

  private: Class(Namespace* outer, SimpleName* simple_name);
  private: ~Class() override;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace hir
}  // namespace elang

#endif // !defined(INCLUDE_elang_hir_class_h)
