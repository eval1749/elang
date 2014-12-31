// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_NAMESPACE_H_
#define ELANG_HIR_NAMESPACE_H_

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
  NamespaceMember* FindMember(AtomicString* simple_name);

 protected:
  Namespace(Namespace* outer, AtomicString* simple_name);
  ~Namespace() override;

 private:
  // NamespaceMember
  Namespace* ToNamespace() override;

  std::unordered_map<AtomicString*, NamespaceMember*> map_;

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_NAMESPACE_H_
