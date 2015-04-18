// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/base/iterator_on_iterator.h"
#include "gtest/gtest.h"

namespace elang {
namespace {

typedef std::vector<int> IntVector;

class MyCollection {
 public:
  class Iterator
      : public IteratorOnIterator<Iterator, IntVector::const_iterator> {
   public:
    explicit Iterator(const IntVector::const_iterator& iterator)
        : IteratorOnIterator(iterator) {}
    ~Iterator() = default;

    Iterator& operator=(const Iterator& other) = default;

    int operator->() const { return operator*(); }
    int operator*() const { return *(*iterator()) * 2; }
  };

  explicit MyCollection(const IntVector& data) : data_(&data) {}
  MyCollection(const MyCollection& other) = default;
  ~MyCollection() = default;

  MyCollection& operator=(const MyCollection& other) = default;

  Iterator begin() const { return Iterator(data_->begin()); }
  Iterator end() const { return Iterator(data_->end()); }

 private:
  const IntVector* data_;
};

TEST(IteratorOnIterator, All) {
  std::vector<int> inputs{1, 2, 3};
  std::vector<int> results;
  for (auto output : MyCollection(inputs))
    results.push_back(output);
  EXPECT_EQ(2, results[0]);
  EXPECT_EQ(4, results[1]);
  EXPECT_EQ(6, results[2]);
}

}  // namespace
}  // namespace elang
