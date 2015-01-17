// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_CLASS_ANALYZER_H_
#define ELANG_COMPILER_ANALYZE_CLASS_ANALYZER_H_

#include <unordered_map>

#include "elang/compiler/analyze/analyzer.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {

class CompilationSession;
class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// ClassAnalyzer
//
class ClassAnalyzer final : public Analyzer, private ast::Visitor {
 public:
  explicit ClassAnalyzer(NameResolver* name_resolver);
  ~ClassAnalyzer() final;

  // The Entry Point of |ClassAnalyzer|, returns true if resolution succeeded,
  // otherwise false.
  bool Run();

 private:
  // ast::Visitor
  void VisitClassBody(ast::ClassBody* node) final;
  void VisitEnum(ast::Enum* node) final;
  void VisitField(ast::Field* node) final;
  void VisitMethod(ast::Method* node) final;
  void VisitNamespaceBody(ast::NamespaceBody* node) final;

  DISALLOW_COPY_AND_ASSIGN(ClassAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_CLASS_ANALYZER_H_
