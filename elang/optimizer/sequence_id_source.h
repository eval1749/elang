// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_SEQUENCE_ID_SOURCE_H_
#define ELANG_OPTIMIZER_SEQUENCE_ID_SOURCE_H_

#include "base/macros.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// SequenceIdSource
//
class SequenceIdSource final {
 public:
  SequenceIdSource();
  ~SequenceIdSource();

  size_t last_id() const { return last_id_; }
  size_t NextId();

 private:
  size_t last_id_;

  DISALLOW_COPY_AND_ASSIGN(SequenceIdSource);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_SEQUENCE_ID_SOURCE_H_
