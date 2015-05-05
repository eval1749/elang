// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>
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
                         const AtomicString& atomic_string) {
  return ostream << base::UTF16ToUTF8(atomic_string.string().as_string());
}

std::ostream& operator<<(std::ostream& ostream,
                         const AtomicString* atomic_string) {
  if (!atomic_string)
    return ostream << "nil";
  return ostream << *atomic_string;
}

}  // namespace elang
