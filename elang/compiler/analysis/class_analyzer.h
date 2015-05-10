// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_

#include <memory>
#include <vector>

#include "elang/base/simple_directed_graph.h"
#include "elang/compiler/analysis/analyzer.h"

namespace elang {
namespace compiler {
class CompilationSession;
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
class ClassAnalyzer final : public Analyzer {
 public:
  explicit ClassAnalyzer(NameResolver* name_resolver);
  ~ClassAnalyzer() final;

  // The Entry Point of |ClassAnalyzer|, returns true if resolution succeeded,
  // otherwise false.
  bool Run();

 private:
  class Collector;

  void AddDependency(ast::Node* from, ast::Node* to);
  void Postpone(ast::Node* node);

  SimpleDirectedGraph<ast::Node*> dependency_graph_;
  std::vector<ast::Node*> pending_nodes_;

  DISALLOW_COPY_AND_ASSIGN(ClassAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_CLASS_ANALYZER_H_
