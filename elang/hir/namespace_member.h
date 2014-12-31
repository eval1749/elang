// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_NAMESPACE_MEMBER_H_
#define ELANG_HIR_NAMESPACE_MEMBER_H_

#include "elang/hir/node.h"

namespace elang {
namespace hir {

class Namespace;
class AtomicString;

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
class NamespaceMember : public Node {
  DECLARE_HIR_NODE_CLASS(NamespaceMember, Node);

 public:
  Namespace* outer() const { return outer_; }
  AtomicString* simple_name() const { return simple_name_; }

  bool IsDescendantOf(const NamespaceMember* other) const;
  virtual Namespace* ToNamespace();

 protected:
  NamespaceMember(Namespace* owner, AtomicString* simple_name);

 private:
  Namespace* const outer_;
  AtomicString* const simple_name_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_NAMESPACE_MEMBER_H_
