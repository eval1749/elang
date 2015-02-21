// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_CONFLICT_MAP_H_
#define ELANG_LIR_ANALYSIS_CONFLICT_MAP_H_

#include "base/macros.h"
#include "elang/base/disjoint_sets.h"
#include "elang/lir/lir_export.h"
#include "elang/lir/value.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ConflictMap
//
class ELANG_LIR_EXPORT ConflictMap final {
 public:
  ConflictMap(const ConflictMap& other) = delete;
  ConflictMap(ConflictMap&& other);
  ConflictMap();
  ~ConflictMap();

  ConflictMap& operator=(const ConflictMap& other) = delete;
  ConflictMap& operator=(ConflictMap&&);

  // Returns true if |register1| and |register2| live together.
  bool IsConflict(Value register1, Value register2) const;

 private:
  friend class ConflictMapBuilder;

  DisjointSets<Value> sets_;
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_CONFLICT_MAP_H_
