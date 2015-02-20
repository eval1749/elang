// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/base/disjoint_sets.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

TEST(DisjointSetsTest, Basic) {
  DisjointSets<int> sets;
  sets.MakeSet(1);
  sets.MakeSet(2);
  sets.MakeSet(3);
  sets.MakeSet(4);
  sets.MakeSet(5);
  sets.MakeSet(6);
  sets.MakeSet(7);
  sets.MakeSet(8);

  EXPECT_FALSE(sets.InSameSet(1, 2));
  EXPECT_FALSE(sets.InSameSet(2, 3));

  // Before Union's: {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}
  sets.Union(1, 2);
  sets.Union(1, 5);
  sets.Union(6, 8);
  sets.Union(5, 6);
  sets.Union(4, 3);

  // After Union's: {1, 2, 5, 6, 8}, {3, 4}, {7}
  EXPECT_TRUE(sets.InSameSet(1, 2));
  EXPECT_TRUE(sets.InSameSet(2, 1));
  EXPECT_TRUE(sets.InSameSet(1, 5));
  EXPECT_TRUE(sets.InSameSet(5, 6));
  EXPECT_TRUE(sets.InSameSet(1, 6));
  EXPECT_TRUE(sets.InSameSet(6, 1));
  EXPECT_TRUE(sets.InSameSet(1, 8));
  EXPECT_TRUE(sets.InSameSet(8, 1));

  EXPECT_TRUE(sets.InSameSet(3, 4));
  EXPECT_TRUE(sets.InSameSet(4, 3));

  EXPECT_TRUE(sets.InSameSet(7, 7));

  EXPECT_FALSE(sets.InSameSet(1, 7));
  EXPECT_FALSE(sets.InSameSet(7, 1));

  EXPECT_FALSE(sets.InSameSet(1, 4));
  EXPECT_FALSE(sets.InSameSet(4, 1));

  EXPECT_FALSE(sets.InSameSet(3, 7));
  EXPECT_FALSE(sets.InSameSet(7, 3));

  auto const sets2 = std::move(sets);
  EXPECT_TRUE(sets2.InSameSet(8, 1));
}

}  // namespace
}  // namespace elang
