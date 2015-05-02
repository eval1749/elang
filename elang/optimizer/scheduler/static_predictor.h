// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_
#define ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_

#include <memory>

#include "base/macros.h"
#include "elang/api/pass.h"
#include "elang/optimizer/scheduler/edge_profile.h"
#include "elang/optimizer/scheduler/schedule_editor.h"

namespace elang {
namespace optimizer {

class BasicBlock;
class EdgeProfile;
class EdgeProfileEditor;

//////////////////////////////////////////////////////////////////////
//
// StaticPredictor
//
// Estimates control-flow edge frequency based on algorithm in:
//  Branch Prediction for Free
//  Thomas Ball, James R. Larus
//  June 1993
//
// Improving Static Branch Prediction in a Compiler
//  Brian L. Deitrich, Ben-Chung Cheng, Wen-mei W. Hwuy
//  October 1998
//
class StaticPredictor final : public api::Pass, public ScheduleEditor::User {
 public:
  explicit StaticPredictor(api::PassObserver* observer, ScheduleEditor* editor);
  ~StaticPredictor();

  std::unique_ptr<EdgeProfile> Run();

 private:
  void Predict(const BasicBlock* from, double frequency);
  void SetBranchFrequency(const BasicBlock* block,
                          const BasicBlock* true_block,
                          const BasicBlock* false_block,
                          double frequency,
                          double probability);
  void SetFrequency(const BasicBlock* from,
                    const BasicBlock* to,
                    double frequency);

  // api::Pass
  base::StringPiece name() const final;
  void DumpAfterPass(const api::PassDumpContext& context) final;

  std::unique_ptr<EdgeProfileEditor> edge_profile_;

  DISALLOW_COPY_AND_ASSIGN(StaticPredictor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_
