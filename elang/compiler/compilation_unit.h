// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_COMPILATION_UNIT_H_
#define ELANG_COMPILER_COMPILATION_UNIT_H_

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

#endif  // ELANG_COMPILER_COMPILATION_UNIT_H_
