// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/base/work_list.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

class Element : public WorkList<Element>::Item {
 public:
  explicit Element(std::string name) : name_(name) {}
  ~Element() = default;

  std::string name() const { return name_; }

 private:
  std::string name_;

  DISALLOW_COPY_AND_ASSIGN(Element);
};

class WorkListTest : public ::testing::Test {
 protected:
  WorkListTest() = default;
  ~WorkListTest() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(WorkListTest);
};

TEST_F(WorkListTest, Basic) {
  WorkList<Element> work_list;
  ASSERT_TRUE(work_list.empty());

  Element element_a("a");
  Element element_b("b");
  Element element_c("c");
  EXPECT_FALSE(work_list.Contains(&element_a));

  work_list.Push(&element_a);
  ASSERT_FALSE(work_list.empty());
  ASSERT_TRUE(work_list.Contains(&element_a));
  EXPECT_EQ(&element_a, work_list.Pop());
  ASSERT_TRUE(work_list.empty());

  work_list.Push(&element_a);
  work_list.Push(&element_b);
  work_list.Push(&element_c);
  EXPECT_EQ(&element_c, work_list.Pop());
  EXPECT_EQ(&element_b, work_list.Pop());
  EXPECT_EQ(&element_a, work_list.Pop());
  EXPECT_TRUE(work_list.empty());
}

}  // namespace
}  // namespace elang
