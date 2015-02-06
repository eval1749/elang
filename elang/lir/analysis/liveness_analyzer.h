// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_
#define ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_

#include <memory>

#include "base/macros.h"
#include "elang/base/liveness_builder.h"

namespace elang {
namespace lir {

class BasicBlock;
class Function;
class LiteralMap;
struct Value;

//////////////////////////////////////////////////////////////////////
//
// LivenessAnalyzer
//
class LivenessAnalyzer final {
 public:
  typedef LivenessCollection<BasicBlock*, Value> Collection;

  explicit LivenessAnalyzer(const LiteralMap* literals);
  ~LivenessAnalyzer();

  std::unique_ptr<Collection> Analyze(Function* function);

 private:
  void Initialize(Function* function);
  void MarkKill(Liveness* liveness, Value value);
  void MarkUse(Liveness* liveness, Value value);

  const LiteralMap* const literals_;
  LivenessBuilder<BasicBlock*, Value> builder_;

  DISALLOW_COPY_AND_ASSIGN(LivenessAnalyzer);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_
