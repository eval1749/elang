// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ORDERED_LIST_H_
#define ELANG_BASE_ORDERED_LIST_H_

#include <unordered_map>
#include <vector>

#include "base/logging.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// OrderedList represents ordered list of |Element|.
//
template <typename Element>
class OrderedList {
 public:
  class Builder {
   public:
    Builder() = default;
    ~Builder() = default;

    void Add(Element element) {
      DCHECK(!list_.map_.count(element));
      list_.map_[element] = static_cast<int>(list_.map_.size());
      list_.vector_.push_back(element);
    }

    OrderedList Get() { return std::move(list_); }

   private:
    OrderedList list_;

    DISALLOW_COPY_AND_ASSIGN(Builder);
  };

  OrderedList(const OrderedList& other) = delete;
  OrderedList(OrderedList&& other)
      : map_(std::move(other.map_)), vector_(std::move(other.vector_)) {}
  OrderedList() = default;
  ~OrderedList() = default;

  OrderedList& operator=(const OrderedList& other) = delete;

  OrderedList& operator=(OrderedList&& other) {
    map_ = std::move(other.map_);
    vector_ = std::move(other.vector_);
    return *this;
  }

  typename std::vector<Element>::const_iterator begin() const {
    return vector_.cbegin();
  }

  typename std::vector<Element>::const_iterator end() const {
    return vector_.cend();
  }

  int position_of(Element value) const {
    auto const it = map_.find(value);
    return it == map_.end() ? -1 : it->second;
  }

  int size() const { return static_cast<int>(vector_.size()); }

 private:
  std::unordered_map<Element, int> map_;
  std::vector<Element> vector_;
};

}  // namespace elang

#endif  // ELANG_BASE_ORDERED_LIST_H_
