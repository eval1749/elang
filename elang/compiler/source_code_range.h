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
  private: int end_offset_;
  private: SourceCode* source_code_;
  private: int start_offset_;

  public: SourceCodeRange(SourceCode* source_code, int start, int end);
  public: SourceCodeRange(const SourceCodeRange& other) = default;
  public: SourceCodeRange();
  public: ~SourceCodeRange();

  public: SourceCodeRange& operator=(const SourceCodeRange& other) = default;

  public: bool operator==(const SourceCodeRange& other) const;
  public: bool operator!=(const SourceCodeRange& other) const;

  public: SourceCodePosition end() const;
  public: SourceCode* source_code() const { return source_code_; }
  public: SourceCodePosition start() const;
  public: int start_offset() const { return start_offset_; }
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_source_code_range_h)

