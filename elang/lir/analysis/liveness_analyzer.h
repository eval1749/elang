// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_
#define ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_

#include <memory>

#include "base/macros.h"

namespace elang {

template <typename Node, typename Variable>
class LivenessCollection;

namespace lir {

class BasicBlock;
class Function;
struct Value;

std::unique_ptr<LivenessCollection<BasicBlock*, Value>>
AnalyzeLiveness(Function* function);

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_ANALYSIS_LIVENESS_ANALYZER_H_
