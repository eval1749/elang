// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/analysis/liveness_builder.h"

namespace elang {

LivenessEditor::LivenessEditor() {
}

LivenessEditor::~LivenessEditor() {
}

Liveness* LivenessEditor::NewLiveness(Zone* zone, int size) {
  return new (zone) Liveness(zone, size);
}

void LivenessEditor::MarkKill(Liveness* liveness, int number) {
  if (number < 0)
    return;
  liveness->kill_.Add(number);
}

void LivenessEditor::MarkUse(Liveness* liveness, int number) {
  if (number < 0)
    return;
  if (liveness->kill_.Contains(number))
    return;
  liveness->in_.Add(number);
}

}  // namespace elang
