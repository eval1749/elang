// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/hir/simple_name.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// SimpleName
//
SimpleName::SimpleName(base::StringPiece16 string)
    : string_(string) {
}

SimpleName::~SimpleName() {
}

}  // namespace hir
}  // namespace elang
