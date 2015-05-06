// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_SHELL_COMPILER_H_
#define ELANG_SHELL_COMPILER_H_

#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/api/pass_controller.h"

namespace base {
class FilePath;
}

namespace elang {
namespace hir {
class Factory;
}
namespace lir {
class Factory;
}
namespace optimizer {
class Factory;
}
namespace compiler {
class CompilationSession;
namespace shell {
class PassRecord;

//////////////////////////////////////////////////////////////////////
//
// Compiler
//
class Compiler final : public api::PassController {
 public:
  explicit Compiler(const std::vector<base::string16>& args);
  ~Compiler();

  // Add source file as compilation unit.
  void AddSourceFile(const base::FilePath& file_path);

  // Run |Main| method with command line arguments.
  int CompileAndGo();

 private:
  typedef CompilationSession CompilationSession;

  CompilationSession* session() { return session_.get(); }

  void CompileAndGoInternal();

  // Report compilation errors so far.
  bool ReportCompileErrors();
  bool ReportHirErrors(const hir::Factory* factory);
  bool ReportIrErrors(const optimizer::Factory* factory);
  bool ReportLirErrors(const lir::Factory* factory);

  // api::PassController implementation
  void DidEndPass(api::Pass* pass) final;
  bool DidStartPass(api::Pass* pass) final;

  const std::vector<base::string16> args_;
  bool dumped_;
  int exit_code_;
  std::unordered_set<std::string> dump_after_passes_;
  std::unordered_set<std::string> dump_before_passes_;
  std::unordered_set<std::string> graph_after_passes_;
  std::unordered_set<std::string> graph_before_passes_;
  std::vector<std::unique_ptr<PassRecord>> pass_records_;
  std::vector<PassRecord*> pass_stack_;
  std::unique_ptr<CompilationSession> session_;
  bool stop_;
  std::string stop_before_;
  std::string stop_after_;

  DISALLOW_COPY_AND_ASSIGN(Compiler);
};

}  // namespace shell
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_SHELL_COMPILER_H_
