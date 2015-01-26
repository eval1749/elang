// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_METHOD_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_METHOD_ANALYZER_H_

#include <unordered_map>

#include "elang/compiler/analysis/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {

class CompilationSession;
class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// MethodAnalyzer
//
class MethodAnalyzer final : public Analyzer, private ast::Visitor {
 public:
  explicit MethodAnalyzer(NameResolver* name_resolver);
  ~MethodAnalyzer() final;

  // The Entry Point of |MethodAnalyzer|, returns true if resolution succeeded,
  // otherwise false.
  bool Run();

 private:
  // ast::Visitor
  void VisitMethod(ast::Method* node) final;

  DISALLOW_COPY_AND_ASSIGN(MethodAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_METHOD_ANALYZER_H_
