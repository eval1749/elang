// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_

#include "base/macros.h"

namespace elang {
namespace compiler {

class CompilationSession;
class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// NamespaceAnalyzer
//
class NamespaceAnalyzer final {
 public:
  explicit NamespaceAnalyzer(NameResolver* resolver);
  ~NamespaceAnalyzer();

  // The entry point of |NamespaceAnalyzer|.
  void Run();

 private:
  CompilationSession* session() const;

  NameResolver* const resolver_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceAnalyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_NAMESPACE_ANALYZER_H_
