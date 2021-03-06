// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/analysis/use_def_list.h"

#include "base/logging.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// UseDefList::Users
//
UseDefList::Users::Users(Zone* zone) : users_(zone) {
}

//////////////////////////////////////////////////////////////////////
//
// UseDefList
//
UseDefList::UseDefList(UseDefList&& other)
    : ZoneOwner(std::move(static_cast<ZoneOwner&&>(other))),
      map_(std::move(other.map_)) {
}

UseDefList::UseDefList() {
}

UseDefList::~UseDefList() {
}

UseDefList& UseDefList::operator=(UseDefList&& other) {
  *static_cast<ZoneOwner*>(this) = std::move(static_cast<ZoneOwner&&>(other));
  map_ = std::move(other.map_);
  return *this;
}

const UseDefList::Users& UseDefList::UsersOf(Value value) const {
  DCHECK(value.is_virtual());
  auto const it = map_.find(value);
  DCHECK(it != map_.end());
  return *it->second;
}

}  // namespace lir
}  // namespace elang
