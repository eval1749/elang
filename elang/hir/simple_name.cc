// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/simple_name.h"

#include "base/strings/utf_string_conversions.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// SimpleName
//
SimpleName::SimpleName(base::StringPiece16 string) : string_(string) {
}

std::ostream& operator<<(std::ostream& ostream, const SimpleName& simple_name) {
  return ostream << base::UTF16ToUTF8(simple_name.string().as_string());
}

}  // namespace hir
}  // namespace elang
