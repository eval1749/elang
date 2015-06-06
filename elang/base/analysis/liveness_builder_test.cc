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
  std::ostringstream ostream;
  ostream << liveness;
  return ostream.str();
}

// Test cases...
TEST(LivenessBuilderTest, Basic) {
  MyLivenessBuilder builder;
  builder.AddVariable(0);
  builder.AddVariable(1);
  builder.AddVariable(2);

  builder.AddNode("bb1");
  builder.MarkKill(builder.Edit("bb1"), 0);

  builder.AddNode("bb2");
  builder.MarkUse(builder.Edit("bb2"), 0);
  builder.MarkKill(builder.Edit("bb2"), 1);
  builder.MarkUse(builder.Edit("bb2"), 1);

  builder.AddNode("bb3");
  builder.MarkUse(builder.Edit("bb3"), 0);
  builder.MarkUse(builder.Edit("bb3"), 1);

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
