// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_namespace_h)
#define INCLUDE_elang_hir_namespace_h

#include <unordered_map>

#include "elang/hir/namespace_member.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Namespace, NamespaceMember);
  friend class Factory;

 public:
  void AddMember(NamespaceMember* member);
  NamespaceMember* FindMember(SimpleName* simple_name);

 protected:
  Namespace(Namespace* outer, SimpleName* simple_name);
  ~Namespace() override;

 private:
  // NamespaceMember
  Namespace* ToNamespace() override;

  std::unordered_map<SimpleName*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace hir
}  // namespace elang

#endif  // !defined(INCLUDE_elang_hir_namespace_h)