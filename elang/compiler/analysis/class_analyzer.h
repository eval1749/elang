// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_

#include <memory>

#include "base/macros.h"

namespace elang {
namespace compiler {
class CompilationSession;
class ConstExprAnalyzer;
class NameResolver;
namespace sm {
class Calculator;
class Editor;
class Enum;
class EnumMember;
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
class ClassAnalyzer final {
 public:
  explicit ClassAnalyzer(NameResolver* name_resolver);
  ~ClassAnalyzer();

  // The Entry Point of |ClassAnalyzer|.
  void Run();

 private:
  const std::unique_ptr<sm::Editor> editor_;
  const std::unique_ptr<ConstExprAnalyzer> analyzer_;

  DISALLOW_COPY_AND_ASSIGN(ClassAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_
