// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_HIR_GENERATOR_H_
#define ELANG_COMPILER_CG_HIR_GENERATOR_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace hir {
class Factory;
class Function;
}
namespace compiler {
class CompilationSession;

//////////////////////////////////////////////////////////////////////
//
// HirGenerator
//
class HirGenerator final : ast::Visitor {
 public:
  HirGenerator(CompilationSession* session, hir::Factory* factory);
  ~HirGenerator();

  void Generate();
  hir::Function* GetMethodFunction(ast::Method* method) const;

 private:
#define V(Name) void Visit##Name(ast::Name* node) final;
  FOR_EACH_AST_NODE(V)
#undef V

  void ProcessMemberContainer(ast::MemberContainer* container);

  hir::Factory* const factory_;
  hir::Function* function_;
  std::unordered_map<ast::Method*, hir::Function*> methods_;
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(HirGenerator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_HIR_GENERATOR_H_
