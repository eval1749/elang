// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/base/ordered_list.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

TEST(OrderedList, All) {
  OrderedList<std::string>::Builder builder;
  builder.Add("A");
  builder.Add("B");
  builder.Add("C");
  auto list = builder.Get();
  EXPECT_EQ(3, list.size());
  EXPECT_EQ(0, list.position_of("A"));
  EXPECT_EQ(1, list.position_of("B"));
  EXPECT_EQ(2, list.position_of("C"));
  EXPECT_EQ(-1, list.position_of("D"));

  std::string result;
  for (auto const element : list) {
    result += element;
  }
  EXPECT_EQ("ABC", result);
}

}  // namespace
}  // namespace elang
