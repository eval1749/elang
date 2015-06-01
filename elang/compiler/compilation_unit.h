// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_COMPILATION_UNIT_H_
#define ELANG_COMPILER_COMPILATION_UNIT_H_

#include <memory>

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class NamespaceBody;
}

class SourceCode;

//////////////////////////////////////////////////////////////////////
//
// CompilationUnit
//
class CompilationUnit {
 public:
  CompilationUnit(ast::NamespaceBody* namespace_body, SourceCode* source_code);
  ~CompilationUnit();

  ast::NamespaceBody* namespace_body() const { return namespace_body_; }
  SourceCode* source_code() const { return source_code_; }

 private:
  ast::NamespaceBody* const namespace_body_;
  SourceCode* const source_code_;

  DISALLOW_COPY_AND_ASSIGN(CompilationUnit);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_UNIT_H_
