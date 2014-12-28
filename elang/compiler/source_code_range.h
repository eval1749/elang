// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_source_code_range_h)
#define INCLUDE_elang_compiler_source_code_range_h

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

#endif  // !defined(INCLUDE_elang_compiler_source_code_range_h)
