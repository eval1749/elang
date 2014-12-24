// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/class.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Class
//
Class::Class(Namespace* outer, SimpleName* simple_name)
    : NamespaceMember(outer, simple_name) {
}

Class::~Class() {
}

}  // namespace hir
}  // namespace elang
