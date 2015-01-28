// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_VARIABLE_ANALYZER_H_
#define ELANG_COMPILER_CG_VARIABLE_ANALYZER_H_

#include <unordered_map>
#include <unordered_set>

#include "base/macros.h"

namespace elang {
class Zone;

namespace hir {
class BasicBlock;
class Function;
class Instruction;
class Value;
}

namespace compiler {
class VariableUsages;

//////////////////////////////////////////////////////////////////////
//
// VariableUsageAnalyze
//
class VariableAnalyzer final {
 public:
  explicit VariableAnalyzer(Zone* result_zone);
  ~VariableAnalyzer();

  // Output from analyzer
  VariableUsages* Analyze();

  // Input for analyzer
  void DidUseVariable(hir::Instruction* home, hir::BasicBlock* block);
  void DidSetVariable(hir::Instruction* home, hir::BasicBlock* block);
  void RegisterFunction(hir::Function* function);
  void RegisterVariable(hir::Instruction* home);

 private:
  struct PerFunctionData {
    std::unordered_set<hir::Instruction*> non_local_reads;
    std::unordered_set<hir::Instruction*> non_local_writes;
  };

  void UpdateVariableUsage(hir::Instruction* home, hir::BasicBlock* block);

  bool did_analyzed_;
  VariableUsages* result_;
  Zone* const result_zone_;
  std::unordered_map<hir::Function*, PerFunctionData*> function_map_;

  DISALLOW_COPY_AND_ASSIGN(VariableAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_VARIABLE_ANALYZER_H_
