// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/source_code_range.h"

#include "base/logging.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/source_code_position.h"

namespace elang {
namespace compiler {

SourceCodeRange::SourceCodeRange(SourceCode* source_code, int start, int end)
    : end_offset_(end), source_code_(source_code), start_offset_(start) {
  DCHECK_LE(start, end);
}

SourceCodeRange::SourceCodeRange()
    : end_offset_(0), source_code_(nullptr), start_offset_(0) {
}

SourceCodeRange::~SourceCodeRange() {
}

bool SourceCodeRange::operator==(const SourceCodeRange& other) const {
  return source_code_ == other.source_code_ &&
         end_offset_ == other.end_offset_ &&
         start_offset_ == other.start_offset_;
}

bool SourceCodeRange::operator!=(const SourceCodeRange& other) const {
  return !operator==(other);
}

SourceCodePosition SourceCodeRange::end() const {
  if (!source_code_)
    return SourceCodePosition(nullptr, end_offset_, 0, end_offset_);
  return source_code_->ComputePosition(end_offset_);
}

SourceCodePosition SourceCodeRange::start() const {
  if (!source_code_)
    return SourceCodePosition(nullptr, start_offset_, 0, start_offset_);
  return source_code_->ComputePosition(start_offset_);
}

}  // namespace compiler
}  // namespace elang
