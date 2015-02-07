// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <string>

#include "elang/base/analysis/liveness_builder.h"
#include "gtest/gtest.h"

namespace elang {

namespace {

typedef LivenessBuilder<std::string, int> MyLivenessBuilder;

std::string ToString(const Liveness& liveness) {
  std::stringstream ostream;
  ostream << liveness;
  return ostream.str();
}

// Test cases...
TEST(LivenessBuilderTest, Basic) {
  MyLivenessBuilder builder;
  builder.AddValue(0);
  builder.AddValue(1);
  builder.AddValue(2);

  builder.AddBlock("bb1");
  builder.MarkKill(builder.LivenessOf("bb1"), 0);

  builder.AddBlock("bb2");
  builder.MarkUse(builder.LivenessOf("bb2"), 0);
  builder.MarkKill(builder.LivenessOf("bb2"), 1);
  builder.MarkUse(builder.LivenessOf("bb2"), 1);

  builder.AddBlock("bb3");
  builder.MarkUse(builder.LivenessOf("bb3"), 0);
  builder.MarkUse(builder.LivenessOf("bb3"), 1);

  auto collection = builder.Finish();
  EXPECT_EQ("{in:{}, out:{}, kill:{0}}",
            ToString(collection->LivenessOf("bb1")));
  EXPECT_EQ("{in:{0}, out:{}, kill:{1}}",
            ToString(collection->LivenessOf("bb2")));
  EXPECT_EQ("{in:{0, 1}, out:{}, kill:{}}",
            ToString(collection->LivenessOf("bb3")));
}

}  // namespace
}  // namespace elang
