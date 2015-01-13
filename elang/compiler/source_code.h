// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SOURCE_CODE_H_
#define ELANG_COMPILER_SOURCE_CODE_H_

#include <map>

#include "base/macros.h"
#include "base/strings/string16.h"

namespace elang {
namespace compiler {

class CharacterStream;
class SourceCodePosition;

//////////////////////////////////////////////////////////////////////
//
// SourceCode
//
class SourceCode {
 public:
  virtual ~SourceCode();

  const base::string16& name() const { return name_; }

  SourceCodePosition ComputePosition(int offset) const;
  virtual CharacterStream* GetStream() = 0;
  void RememberStartOfLine(int offset);

 protected:
  explicit SourceCode(const base::string16& name);

 private:
  int line_number_;
  std::map<int, int> map_;
  const base::string16 name_;

  DISALLOW_COPY_AND_ASSIGN(SourceCode);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SOURCE_CODE_H_
