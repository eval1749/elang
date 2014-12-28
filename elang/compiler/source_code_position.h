// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SOURCE_CODE_POSITION_H_
#define ELANG_COMPILER_SOURCE_CODE_POSITION_H_

namespace elang {
namespace compiler {

class SourceCode;

//////////////////////////////////////////////////////////////////////
//
// SourceCodePosition
//
class SourceCodePosition {
 public:
  SourceCodePosition(SourceCode* source_code,
                     int offset,
                     int line_number,
                     int column_number);
  SourceCodePosition(const SourceCodePosition& other) = default;
  ~SourceCodePosition();

  SourceCodePosition& operator=(const SourceCodePosition& other) = default;

  bool operator==(const SourceCodePosition& other) const;
  bool operator!=(const SourceCodePosition& other) const;
  bool operator<(const SourceCodePosition& other) const;
  bool operator<=(const SourceCodePosition& other) const;
  bool operator>(const SourceCodePosition& other) const;
  bool operator>=(const SourceCodePosition& other) const;

  int column() const { return column_; }
  int line() const { return line_; }
  int offset() const { return offset_; }
  SourceCode* source_code() const { return source_code_; }

 private:
  int column_;
  int line_;
  int offset_;
  SourceCode* source_code_;
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SOURCE_CODE_POSITION_H_
