// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_NAMESPACE_MEMBER_H_
#define ELANG_VM_NAMESPACE_MEMBER_H_

#include "elang/vm/node.h"

namespace elang {
class AtomicString;
namespace vm {

class Namespace;

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
class NamespaceMember : public Node {
  DECLARE_VM_NODE_CLASS(NamespaceMember, Node);

 public:
  Namespace* outer() const { return outer_; }
  AtomicString* name() const { return name_; }

  bool IsDescendantOf(const NamespaceMember* other) const;
  virtual Namespace* ToNamespace();

 protected:
  NamespaceMember(Namespace* owner, AtomicString* name);

 private:
  Namespace* const outer_;
  AtomicString* const name_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceMember);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_NAMESPACE_MEMBER_H_
