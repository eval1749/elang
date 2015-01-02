// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/base/atomic_string.h"

#include "base/strings/utf_string_conversions.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
AtomicString::AtomicString(base::StringPiece16 string) : string_(string) {
}

std::ostream& operator<<(std::ostream& ostream,
                         const AtomicString& simple_name) {
  return ostream << base::UTF16ToUTF8(simple_name.string().as_string());
}

}  // namespace elang
