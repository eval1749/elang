// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/source_code_position.h"

#include "elang/compiler/source_code.h"

namespace elang {
namespace compiler {

SourceCodePosition::SourceCodePosition(SourceCode* source_code,
                                       int offset,
                                       int line,
                                       int column)
    : column_(column), line_(line), offset_(offset), source_code_(source_code) {
}

SourceCodePosition::~SourceCodePosition() {
}

bool SourceCodePosition::operator==(const SourceCodePosition& other) const {
  return source_code_ == other.source_code_ && offset_ == other.offset_;
}

bool SourceCodePosition::operator!=(const SourceCodePosition& other) const {
  return !operator==(other);
}

bool SourceCodePosition::operator<(const SourceCodePosition& other) const {
  if (source_code_ == other.source_code_)
    return offset_ < other.offset_;
  return source_code_->name() < other.source_code_->name();
}

bool SourceCodePosition::operator<=(const SourceCodePosition& other) const {
  return operator<(other) || operator==(other);
}

bool SourceCodePosition::operator>(const SourceCodePosition& other) const {
  return !operator<=(other);
}

bool SourceCodePosition::operator>=(const SourceCodePosition& other) const {
  return !operator<(other);
}

}  // namespace compiler
}  // namespace elang
