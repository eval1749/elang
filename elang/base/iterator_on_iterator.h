// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ITERATOR_ON_ITERATOR_H_
#define ELANG_BASE_ITERATOR_ON_ITERATOR_H_

namespace elang {

// IteratorOnIterator provides functions for implementing iterator on
// another iterator.
template <class Derived, class BaseIterator>
class IteratorOnIterator {
 public:
  IteratorOnIterator& operator=(const IteratorOnIterator& other) = default;

  Derived& operator++() {
    ++iterator_;
    return *static_cast<Derived*>(this);
  }

  bool operator==(const IteratorOnIterator& other) const {
    return iterator_ == other.iterator_;
  }

  bool operator!=(const IteratorOnIterator& other) const {
    return !operator==(other);
  }

 protected:
  explicit IteratorOnIterator(const BaseIterator& iterator)
      : iterator_(iterator) {}
  ~IteratorOnIterator() = default;

  const BaseIterator* iterator() const { return &iterator_; }
  BaseIterator* iterator() { return &iterator_; }

 private:
  BaseIterator iterator_;
};

}  // namespace elang

#endif  // ELANG_BASE_ITERATOR_ON_ITERATOR_H_
