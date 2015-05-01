// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_
#define ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_

#include <memory>
#include <map>
#include <utility>
#include <vector>

#include "base/macros.h"
#include "elang/api/pass.h"
#include "elang/optimizer/scheduler/schedule_editor.h"

namespace elang {
namespace optimizer {

class BasicBlock;

//////////////////////////////////////////////////////////////////////
//
// EdgeFrequencyMap
//
class EdgeFrequencyMap final {
 public:
  class Editor {
   public:
    Editor();
    ~Editor();

    void Add(const BasicBlock* from, const BasicBlock* to, double frequency);
    std::unique_ptr<EdgeFrequencyMap> Finish();
    double FrequencyOf(const BasicBlock* form, const BasicBlock* to) const;
    bool Has(const BasicBlock* from, const BasicBlock* to) const;

   private:
    std::unique_ptr<EdgeFrequencyMap> edge_map_;

    DISALLOW_COPY_AND_ASSIGN(Editor);
  };

  using Map = std::map<std::pair<const BasicBlock*, const BasicBlock*>, double>;

  EdgeFrequencyMap();
  ~EdgeFrequencyMap();

  const Map& all_edges() const { return map_; }
  size_t size() const { return map_.size(); }

  double FrequencyOf(const BasicBlock* form, const BasicBlock* to) const;

 private:
  friend class Editor;

  Map map_;

  DISALLOW_COPY_AND_ASSIGN(EdgeFrequencyMap);
};

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

  std::unique_ptr<EdgeFrequencyMap> Run();

 private:
  // api::Pass
  base::StringPiece name() const final;
  void DumpPass(const api::PassDumpContext& context) final;

  void Predict(const BasicBlock* from, double frequency);
  void SetFrequency(const BasicBlock* from,
                    const BasicBlock* to,
                    double frequency);

  EdgeFrequencyMap::Editor edge_map_;

  DISALLOW_COPY_AND_ASSIGN(StaticPredictor);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SCHEDULER_STATIC_PREDICTOR_H_
