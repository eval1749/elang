// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_compilation_unit_h)
#define elang_compiler_compilation_unit_h

#include "base/macros.h"

namespace elang {
namespace compiler {

class SourceCode;

class CompilationUnit {
  private: SourceCode* source_code_;

  public: CompilationUnit(SourceCode* source_code);
  public: ~CompilationUnit();

  public: SourceCode* source_code() const { return source_code_; }

  DISALLOW_COPY_AND_ASSIGN(CompilationUnit);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_compilation_unit_h)

