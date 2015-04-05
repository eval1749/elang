// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <iterator>
#include <vector>

#include "base/at_exit.h"
#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "elang/shell/compiler.h"

namespace elang {
namespace compiler {
namespace shell {

//////////////////////////////////////////////////////////////////////
//
// Main - The entry point.
//
extern "C" int main() {
  base::AtExitManager at_exit;
  base::CommandLine::set_slash_is_not_a_switch();
  base::CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }

  auto const command_line = base::CommandLine::ForCurrentProcess();
  Compiler compiler(command_line->GetArgs());

  for (auto file_name : command_line->GetArgs()) {
    base::FilePath file_path(file_name);
    compiler.AddSourceFile(base::MakeAbsoluteFilePath(file_path));
  }
  return compiler.CompileAndGo();
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
