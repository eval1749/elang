// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/source_code.h"

#include "base/logging.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// SourceCode
//
SourceCode::SourceCode(const base::string16& name)
    : line_number_(0), name_(name) {
  map_[0] = 0;
}

SourceCode::~SourceCode() {
}

SourceCodePosition SourceCode::ComputePosition(int offset) const {
  DCHECK(!map_.empty());
  auto const source_code = const_cast<SourceCode*>(this);
  if (!offset)
    return SourceCodePosition(source_code, offset, 0, 0);
  auto it = map_.lower_bound(offset);
  if (it == map_.end()) {
    auto const line_offset = map_.rbegin()->first;
    DCHECK_LE(line_offset, offset);
    auto const line_number = map_.rbegin()->second;
    return SourceCodePosition(source_code, offset, line_number,
                              offset - line_offset);
  }
  if (it->first == offset)
    return SourceCodePosition(source_code, offset, it->second, 0);
  DCHECK(it != map_.begin());
  --it;
  auto const line_offset = it->first;
  DCHECK_LE(line_offset, offset);
  auto const line_number = it->second;
  return SourceCodePosition(source_code, offset, line_number,
                            offset - line_offset);
}

void SourceCode::RememberStartOfLine(int position) {
  map_[position] = line_number_;
  ++line_number_;
}

}  // namespace compiler
}  // namespace elang
