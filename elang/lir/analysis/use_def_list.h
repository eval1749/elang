// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_USE_DEF_LIST_H_
#define ELANG_LIR_ANALYSIS_USE_DEF_LIST_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_vector.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

class Instruction;

//////////////////////////////////////////////////////////////////////
//
// UseDefList
//
class ELANG_LIR_EXPORT UseDefList final : public ZoneOwner {
 public:
  class Users final : public ZoneAllocated {
   public:
    ~Users() = delete;

    ZoneVector<Instruction*>::const_iterator begin() const {
      return users_.begin();
    }

    bool empty() const { return users_.empty(); }

    ZoneVector<Instruction*>::const_iterator end() const {
      return users_.end();
    }

   private:
    friend class UseDefListBuilder;

    explicit Users(Zone* zone);

    ZoneVector<Instruction*> users_;
  };

  UseDefList(const UseDefList& other) = delete;
  UseDefList(UseDefList&& other);
  UseDefList();
  ~UseDefList();

  UseDefList& operator=(const UseDefList& other) = delete;
  UseDefList& operator=(UseDefList&& other);

  const Users& UsersOf(Value value) const;

 private:
  friend class UseDefListBuilder;

  std::unordered_map<Value, Users*> map_;
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_USE_DEF_LIST_H_
