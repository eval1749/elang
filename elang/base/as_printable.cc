// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/as_printable.h"

namespace elang {

std::ostream& operator<<(std::ostream& ostream, const AsPrintable& printable) {
  static char const xdigits[] = "0123456789ABCDEF";
  static char const escapes[] = "0------abtnvfr";
  auto const ch = printable.data;
  char buffer[7];
  if (ch <= 0x0D && escapes[ch] != '-') {
    buffer[0] = '\\';
    buffer[1] = escapes[ch];
    buffer[2] = 0;
  } else if (ch == printable.delimiter || ch == '\\') {
    buffer[0] = '\\';
    buffer[1] = ch;
    buffer[2] = 0;
  } else if (ch < ' ' || ch >= 0x7F) {
    buffer[0] = '\\';
    buffer[1] = 'u';
    buffer[2] = xdigits[(ch >> 12) & 15];
    buffer[3] = xdigits[(ch >> 8) & 15];
    buffer[4] = xdigits[(ch >> 4) & 15];
    buffer[5] = xdigits[ch & 15];
    buffer[6] = 0;
  } else {
    buffer[0] = ch;
    buffer[1] = 0;
  }
  return ostream << buffer;
}

}  // namespace elang
