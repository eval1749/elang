// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_WORK_LIST_H_
#define ELANG_BASE_WORK_LIST_H_

#include "base/macros.h"
#include "base/logging.h"

namespace elang {

template <typename Element>
class WorkList {
 public:
  class Item {
   protected:
#ifndef NDEBUG
    Item() : previous_element_(nullptr), work_list_(nullptr) {}
#else
    Item() : previous_element_(nullptr) {}
#endif
    ~Item() = default;

   private:
    friend class WorkList;

    Element* previous_element_;
#ifndef NDEBUG
    WorkList* work_list_;
#endif
    DISALLOW_COPY_AND_ASSIGN(Item);
  };

  WorkList() : last_element_(nullptr) {}
  ~WorkList() { DCHECK(empty()); }

  bool empty() const { return !last_element_; }

  bool Contains(const Element* element) const;
  Element* Pop();
  void Push(Element* element);

 private:
  Element* last_element_;

  DISALLOW_COPY_AND_ASSIGN(WorkList);
};

#ifndef NDEBUG
template <typename Element>
bool WorkList<Element>::Contains(const Element* element) const {
  return !!static_cast<const Item*>(element)->previous_element_;
}
#else
template <typename Element>
bool WorkList<Element>::Contains(const Element* element) const {
  auto const item = static_cast<Item*>(element);
  if (!item->work_list)
    return false;
  DCHECK_EQ(item->work_list, this);
  return true;
}
#endif

template <typename Element>
Element* WorkList<Element>::Pop() {
  DCHECK(!empty());
  auto const element = last_element_;
  auto const item = static_cast<Item*>(element);
  last_element_ = item->previous_element_;
  if (last_element_ == element)
    last_element_ = nullptr;
  item->previous_element_ = nullptr;
#ifndef NDEBUG
  item->work_list_ = nullptr;
#endif
  return element;
}

template <typename Element>
void WorkList<Element>::Push(Element* element) {
  DCHECK(!Contains(element));
  auto const item = static_cast<Item*>(element);
#ifndef NDEBUG
  item->work_list_ = this;
#endif
  item->previous_element_ = last_element_ ? last_element_ : element;
  last_element_ = element;
}

}  // namespace elang

#endif  // ELANG_BASE_WORK_LIST_H_
