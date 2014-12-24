// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_source_code_position_h)
#define INCLUDE_elang_compiler_source_code_position_h

namespace elang {
namespace compiler {

class SourceCode;

//////////////////////////////////////////////////////////////////////
//
// SourceCodePosition
//
class SourceCodePosition {
  private: int column_;
  private: int line_;
  private: int offset_;
  private: SourceCode* source_code_;

  public: SourceCodePosition(SourceCode* source_code, int offset,
                             int line_number, int column_number);
  public: SourceCodePosition(const SourceCodePosition& other) = default;
  public: ~SourceCodePosition();

  public: SourceCodePosition& operator=(
      const SourceCodePosition& other) = default;

  public: bool operator==(const SourceCodePosition& other) const;
  public: bool operator!=(const SourceCodePosition& other) const;
  public: bool operator<(const SourceCodePosition& other) const;
  public: bool operator<=(const SourceCodePosition& other) const;
  public: bool operator>(const SourceCodePosition& other) const;
  public: bool operator>=(const SourceCodePosition& other) const;

  public: int column() const { return column_; }
  public: int line() const { return line_; }
  public: int offset() const { return offset_; }
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_source_code_position_h)

