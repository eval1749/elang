// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>
#include <sstream>

#include "elang/base/as_printable.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

TEST(AsPrintable, All) {
  std::ostringstream ostream;
  auto const delimiter = '|';
  ostream << AsPrintable('\0', delimiter);
  for (auto const ch : std::string("xyz\a\b\t\n\v\f\r|"))
    ostream << AsPrintable(ch, delimiter);
  ostream << AsPrintable(0x1234, delimiter);
  ostream << AsPrintable(0xABCD, delimiter);
  EXPECT_EQ("\\0xyz\\a\\b\\t\\n\\v\\f\\r\\|\\u1234\\uABCD", ostream.str());
}

}  // namespace
}  // namespace elang
