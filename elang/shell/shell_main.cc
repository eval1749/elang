// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "base/at_exit.h"
#include "base/command_line.h"
#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/compiler/string_source_code.h"

namespace elang {
namespace shell {

extern "C" int main() {
  base::AtExitManager at_exit;
  CommandLine::set_slash_is_not_a_switch();
  CommandLine::Init(0, nullptr);
  {
    logging::LoggingSettings settings;
    settings.logging_dest = logging::LOG_TO_SYSTEM_DEBUG_LOG;
    logging::InitLogging(settings);
  }

  compiler::CompilationSession session;
  auto command_line = base::CommandLine::ForCurrentProcess();
  for (auto file_name : command_line->GetArgs()) {
    std::cout << "Compile: " << file_name << std::endl;
    compiler::StringSourceCode source_code(file_name, L"namespace foo");
    auto const compilation_unit = session.NewCompilationUnit(&source_code);
    compiler::Parser parser(&session, compilation_unit);
    parser.Run();
  }

  return 0;
}

}  // namespace shell
}  // namespace elang
