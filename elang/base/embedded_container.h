// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_EMBEDDED_CONTAINER_H_
#define ELANG_BASE_EMBEDDED_CONTAINER_H_

#include "base/logging.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// EmbeddedContainer
//
template <typename ElementType, int NumElements>
class EmbeddedContainer {
 public:
  EmbeddedContainer() : elements_() {}

  const ElementType& operator[](int index) const {
    DCHECK_LT(index, length());
    return elements_[index];
  }

  ElementType& operator[](int index) {
    DCHECK_LT(index, length());
    return elements_[index];
  }

  int length() const { return NumElements; }

 private:
  ElementType elements_[NumElements];
};

template <typename ElementType>
class EmbeddedContainer<ElementType, 0> {
 public:
  const ElementType& operator[](int index) const {
    NOTREACHED();
    static ElementType sentinel;
    return sentinel;
  }

  ElementType& operator[](int index) {
    NOTREACHED();
    static ElementType sentinel;
    return sentinel;
  }
};

}  // namespace elang

#endif  // ELANG_BASE_EMBEDDED_CONTAINER_H_
