// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/namespace_member.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// NamespaceMember
//
NamespaceMember::NamespaceMember(Namespace* outer, SimpleName* simple_name)
    : outer_(outer), simple_name_(simple_name) {
}

NamespaceMember::~NamespaceMember() {
}

}  // namespace hir
}  // namespace elang
