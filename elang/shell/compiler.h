// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_COMPILER_H_
#define ELANG_SHELL_COMPILER_H_

#include <memory>

#include "base/macros.h"

namespace base {
class FilePath;
}

namespace elang {
namespace compiler {
class CompilationSession;
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// Compiler
//
class Compiler final {
 public:
  Compiler();
  ~Compiler();

  void set_dump_hir(bool dump) { is_dump_hir_ = dump; }
  void set_dump_lir(bool dump) { is_dump_lir_ = dump; }

  // Add source file as compilation unit.
  void AddSourceFile(const base::FilePath& file_path);

  // Run |Main| method with command line arguments.
  int CompileAndGo();

  // Report compilation errors so far.
  void ReportErrors();

 private:
  typedef CompilationSession CompilationSession;

  CompilationSession* session() { return session_.get(); }

  bool is_dump_hir_;
  bool is_dump_lir_;
  std::unique_ptr<CompilationSession> session_;

  DISALLOW_COPY_AND_ASSIGN(Compiler);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_COMPILER_H_
