// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SOURCE_CODE_RANGE_H_
#define ELANG_COMPILER_SOURCE_CODE_RANGE_H_

namespace elang {
namespace compiler {

class SourceCode;
class SourceCodePosition;

//////////////////////////////////////////////////////////////////////
//
// SourceCodeRange
//
class SourceCodeRange {
 public:
  SourceCodeRange(SourceCode* source_code, int start, int end);
  SourceCodeRange(const SourceCodeRange& other) = default;
  SourceCodeRange();
  ~SourceCodeRange();

  SourceCodeRange& operator=(const SourceCodeRange& other) = default;

  bool operator==(const SourceCodeRange& other) const;
  bool operator!=(const SourceCodeRange& other) const;

  SourceCodePosition end() const;
  int end_offset() const { return end_offset_; }
  SourceCode* source_code() const { return source_code_; }
  SourceCodePosition start() const;
  int start_offset() const { return start_offset_; }

 private:
  int end_offset_;
  SourceCode* source_code_;
  int start_offset_;
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SOURCE_CODE_RANGE_H_
