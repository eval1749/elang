// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_NAMESPACE_H_
#define ELANG_VM_NAMESPACE_H_

#include "elang/base/zone_unordered_map.h"
#include "elang/vm/namespace_member.h"

namespace elang {
namespace vm {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace : public NamespaceMember {
  DECLARE_VM_NODE_CLASS(Namespace, NamespaceMember);

 public:
  void AddMember(NamespaceMember* member);
  NamespaceMember* FindMember(AtomicString* name);

 protected:
  Namespace(Zone* zone, Namespace* outer, AtomicString* name);

 private:
  // NamespaceMember
  Namespace* ToNamespace() override;

  ZoneUnorderedMap<AtomicString*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_NAMESPACE_H_
