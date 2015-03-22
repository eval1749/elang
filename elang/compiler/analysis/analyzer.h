// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_ANALYZER_H_
#define ELANG_COMPILER_ANALYSIS_ANALYZER_H_

#include <unordered_map>

#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {

namespace sm {
class Factory;
class Node;
class Type;
}

class NameResolver;
class Semantics;

//////////////////////////////////////////////////////////////////////
//
// Analyzer
//
class Analyzer : public CompilationSessionUser {
 public:
  NameResolver* name_resolver() const { return name_resolver_; }

 protected:
  explicit Analyzer(NameResolver* resolver);
  virtual ~Analyzer();

  sm::Factory* factory() const { return ir_factory(); }
  sm::Factory* ir_factory() const;
  NameResolver* resolver() const { return name_resolver_; }

  // Shortcut to |NameResolver|.
  sm::Node* Resolve(ast::NamedNode* ast_node);
  sm::Type* ResolveTypeReference(ast::Type* reference,
                                 ast::ContainerNode* container);

 private:
  NameResolver* const name_resolver_;

  DISALLOW_COPY_AND_ASSIGN(Analyzer);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_ANALYZER_H_
