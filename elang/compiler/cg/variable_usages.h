// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_VARIABLE_USAGES_H_
#define ELANG_COMPILER_CG_VARIABLE_USAGES_H_

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_unordered_set.h"
#include "elang/base/zone_vector.h"

namespace elang {

namespace hir {
class BasicBlock;
class Function;
class Instruction;
class Value;
class Type;
}

namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// VariableUsages
//
class VariableUsages final : public ZoneAllocated {
 public:
  class Data final : public ZoneAllocated {
   public:
    explicit Data(hir::Instruction* home);

    // Returns an instruction allocating variable home on heap/stack.
    hir::Instruction* home() const { return home_; }
    bool is_local() const { return used_in_ == UsedIn::SingleBlock; }
    hir::Function* owner() const { return owner_; }
    hir::Type* type() const { return type_; }

   private:
    friend class VariableAnalyzer;
    friend class VariableUsages;

    enum class UsedIn {
      MultipleBlocks,
      NonLocalBlocks,
      SingleBlock,
    };

    hir::Instruction* const home_;
    hir::Function* const owner_;
    hir::Type* const type_;
    UsedIn used_in_;

    DISALLOW_COPY_AND_ASSIGN(Data);
  };

  explicit VariableUsages(Zone* zone);

  Data* data_for(hir::Instruction* home) const;

  const ZoneVector<Data*>& local_variables_of(hir::Function* function) const;

  bool IsAliveAt(hir::Instruction* home, hir::BasicBlock* block) const;

 private:
  friend class VariableAnalyzer;

  struct PerFunctionData : ZoneAllocated {
    ZoneVector<Data*> local_variables;
    ZoneVector<Data*> non_local_reads;
    ZoneVector<Data*> non_local_writes;

    explicit PerFunctionData(Zone* zone);
  };

  ~VariableUsages() = delete;

  ZoneUnorderedMap<hir::Function*, PerFunctionData*> function_map_;
  ZoneUnorderedMap<hir::Instruction*, Data*> variable_map_;

  DISALLOW_COPY_AND_ASSIGN(VariableUsages);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_VARIABLE_USAGES_H_
