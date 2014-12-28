// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_namespace_member_h)
#define INCLUDE_elang_hir_namespace_member_h

#include "elang/hir/node.h"

namespace elang {
namespace hir {

class Namespace;
class SimpleName;

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
class NamespaceMember : public Node {
  DECLARE_CASTABLE_CLASS(NamespaceMember, Node);

 public:
  Namespace* outer() const { return outer_; }
  SimpleName* simple_name() const { return simple_name_; }

  bool IsDescendantOf(const NamespaceMember* other) const;
  virtual Namespace* ToNamespace();

 protected:
  NamespaceMember(Namespace* owner, SimpleName* simple_name);
  ~NamespaceMember() override;

 private:
  Namespace* const outer_;
  SimpleName* const simple_name_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace hir
}  // namespace elang

#endif  // !defined(INCLUDE_elang_hir_namespace_member_h)
