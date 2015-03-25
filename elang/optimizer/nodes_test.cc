// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/testing/optimizer_test.h"

#include "elang/optimizer/nodes.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// NodeTest
//
class NodeTest : public testing::OptimizerTest {
 protected:
  NodeTest() = default;
  ~NodeTest() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(NodeTest);
};

}  // namespace optimizer
}  // namespace elang
