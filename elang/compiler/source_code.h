// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_source_code_h)
#define INCLUDE_elang_compiler_source_code_h

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
  private: int line_number_;
  private: std::map<int, int> map_;
  private: const base::string16 name_;

  protected: SourceCode(const base::string16& name);
  public: virtual ~SourceCode();

  public: const base::string16& name() const { return name_; }

  public: SourceCodePosition ComputePosition(int offset) const;
  public: virtual CharacterStream* GetStream() = 0;
  public: void RememberStartOfLine(int offset);

  DISALLOW_COPY_AND_ASSIGN(SourceCode);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_source_code_h)

