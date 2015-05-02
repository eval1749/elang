// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_PASS_RECORD_H_
#define ELANG_SHELL_PASS_RECORD_H_

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"

namespace elang {
namespace compiler {
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// PassRecord
//
class PassRecord {
 public:
  PassRecord(size_t depth, base::StringPiece name);
  ~PassRecord();

  size_t depth() const { return depth_; }
  base::TimeDelta duration() const;
  base::StringPiece name() const { return name_; }

  void EndMetrics();
  void StartMetrics();

 private:
  base::Time end_at_;
  size_t const depth_;
  const base::StringPiece name_;
  base::Time start_at_;

  DISALLOW_COPY_AND_ASSIGN(PassRecord);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_PASS_RECORD_H_
