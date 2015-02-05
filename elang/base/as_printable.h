// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_AS_PRINTABLE_H_
#define ELANG_BASE_AS_PRINTABLE_H_

#include <ostream>

#include "base/strings/string16.h"
#include "elang/base/base_export.h"

namespace elang {

struct ELANG_BASE_EXPORT PrintableCharacter {
  base::char16 data;
  base::char16 delimiter;

  PrintableCharacter(base::char16 data, base::char16 delimiter)
      : data(data), delimiter(delimiter) {}
};

inline PrintableCharacter AsPrintable(base::char16 data,
                                      base::char16 delimiter) {
  return PrintableCharacter(data, delimiter);
}

ELANG_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const PrintableCharacter& printable);

}  // namespace elang

#endif  // ELANG_BASE_AS_PRINTABLE_H_
