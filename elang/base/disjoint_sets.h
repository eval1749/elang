// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_DISJOINT_SETS_H_
#define ELANG_BASE_DISJOINT_SETS_H_

#include <unordered_map>

#include "base/logging.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// DisjointSets
//
// Example:
//  DisjointSet<int> sets;
//  sets.MarkSet(1);
//  sets.MarkSet(2);
//  sets.MarkSet(3);
//
//  sets.Union(1, 3);
//
//  if (sets.IsSameSet(2, 3))
//   ....
//
template <typename Element>
class DisjointSets : public ZoneOwner {
 public:
  DisjointSets(const DisjointSets& other) = delete;
  DisjointSets(DisjointSets&& other);
  DisjointSets();
  ~DisjointSets() = default;

  DisjointSets& operator=(const DisjointSets& other) = delete;
  DisjointSets& operator=(DisjointSets&& other);

  void MakeSet(Element);
  bool InSameSet(Element element1, Element element2) const;
  void Union(Element element1, Element element2);

 private:
  struct Set : ZoneAllocated {
    Element const element_;
    Set* parent_;
    int rank_;

    explicit Set(Element element)
        : element_(element), parent_(this), rank_(0) {}
    ~Set() = delete;

   private:
    DISALLOW_COPY_AND_ASSIGN(Set);
  };

  Set* Find(Element element) const;
  Set* Get(Element element) const;

  std::unordered_map<Element, Set*> map_;
};

template <typename Element>
DisjointSets<Element>::DisjointSets(DisjointSets&& other)
    : ZoneOwner(std::move(static_cast<ZoneOwner&&>(other))),
      map_(std::move(other.map_)) {
}

template <typename Element>
DisjointSets<Element>::DisjointSets() {
}

template <typename Element>
DisjointSets<Element>& DisjointSets<Element>::operator=(DisjointSets&& other) {
  *static_cast<ZoneOwner*>(this) = std::move(static_cast<ZoneOwner&&>(other));
  map_ = std::move(other.map_);
  return *this;
}

template <typename Element>
typename DisjointSets<Element>::Set* DisjointSets<Element>::Find(
    Element element) const {
  auto const set = Get(element);
  if (set->parent_ != set)
    set->parent_ = Find(set->parent_->element_);
  return set->parent_;
}

template <typename Element>
typename DisjointSets<Element>::Set* DisjointSets<Element>::Get(
    Element element) const {
  auto const it = map_.find(element);
  DCHECK(it != map_.end());
  return it->second;
}

template <typename Element>
bool DisjointSets<Element>::InSameSet(Element element1,
                                      Element element2) const {
  return Find(element1) == Find(element2);
}

template <typename Element>
void DisjointSets<Element>::MakeSet(Element element) {
  DCHECK(!map_.count(element));
  map_[element] = new (zone()) Set(element);
}

template <typename Element>
void DisjointSets<Element>::Union(Element element1, Element element2) {
  auto const set1 = Find(element1);
  auto const set2 = Find(element2);
  if (set1 == set2)
    return;
  if (set1->rank_ < set2->rank_) {
    set1->parent_ = set2;
    return;
  }
  if (set1->rank_ > set2->rank_) {
    set2->parent_ = set1;
    return;
  }
  set2->parent_ = set1;
  ++set1->rank_;
}

}  // namespace elang

#endif  // ELANG_BASE_DISJOINT_SETS_H_
