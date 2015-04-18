// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>

#include "elang/base/bit_set.h"
#include "elang/base/zone_owner.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

class BitSetTest : public ::testing::Test, public ZoneOwner {
 protected:
  BitSetTest() = default;
  ~BitSetTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(BitSetTest);
};

TEST_F(BitSetTest, Basic) {
  auto const set1 = new (zone()) BitSet(zone(), 100);
  EXPECT_TRUE(set1->IsEmpty());

  auto const set2 = new (zone()) BitSet(zone(), 100);
  EXPECT_TRUE(set1->Equals(*set2));

  set1->Add(50);
  EXPECT_TRUE(set1->Contains(50));
  EXPECT_FALSE(set1->IsEmpty());
  EXPECT_FALSE(set1->Equals(*set2));

  set1->Remove(50);
  EXPECT_TRUE(set1->Equals(*set2));
  EXPECT_TRUE(set1->IsEmpty());

  set1->Add(30);
  set1->Add(60);
  set1->Add(90);
  set2->Union(*set1);
  EXPECT_TRUE(set2->Contains(30));
  EXPECT_TRUE(set2->Contains(60));
  EXPECT_TRUE(set2->Contains(90));

  set1->Clear();
  EXPECT_TRUE(set1->IsEmpty());
  set1->Add(60);
  set1->Add(90);
  set2->Intersect(*set1);
  EXPECT_FALSE(set2->Contains(30));
  EXPECT_TRUE(set2->Contains(60));
  EXPECT_TRUE(set2->Contains(90));

  set1->Remove(60);
  set2->Subtract(*set1);
  EXPECT_FALSE(set2->Contains(30));
  EXPECT_TRUE(set2->Contains(60));
  EXPECT_FALSE(set2->Contains(90));
}

TEST_F(BitSetTest, Printer) {
  auto const set1 = new (zone()) BitSet(zone(), 10);
  set1->Add(1);
  set1->Add(2);
  set1->Add(5);
  set1->Add(7);
  set1->Add(9);
  std::stringstream ostream;
  ostream << *set1;
  EXPECT_EQ("{1, 2, 5, 7, 9}", ostream.str());
}

}  // namespace
}  // namespace elang
