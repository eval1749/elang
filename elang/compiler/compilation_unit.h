// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_compilation_unit_h)
#define INCLUDE_elang_compiler_compilation_unit_h

#include <memory>

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class Namespace;
}

class CompilationSession;
class SourceCode;

//////////////////////////////////////////////////////////////////////
//
// CompilationUnit
//
class CompilationUnit {
 public:
  CompilationUnit(CompilationSession* session, SourceCode* source_code);
  ~CompilationUnit();

  SourceCode* source_code() const { return source_code_; }

 private:
  SourceCode* const source_code_;

  DISALLOW_COPY_AND_ASSIGN(CompilationUnit);
};

}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_compilation_unit_h)

