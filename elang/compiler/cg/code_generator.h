// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_CODE_GENERATOR_H_
#define ELANG_COMPILER_CG_CODE_GENERATOR_H_

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
class NameResolver;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// CodeGenerator
//
class CodeGenerator final : ast::Visitor {
 public:
  CodeGenerator(CompilationSession* session,
               hir::Factory* factory,
               NameResolver* name_resolver);
  ~CodeGenerator();

  void Generate();
  hir::Function* GetMethodFunction(ast::Method* method) const;

 private:
#define V(Name) void Visit##Name(ast::Name* node) final;
  FOR_EACH_CONCRETE_AST_NODE(V)
#undef V

  hir::Factory* const factory_;
  hir::Function* function_;
  std::unordered_map<ast::Method*, hir::Function*> methods_;
  NameResolver* name_resolver_;
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(CodeGenerator);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_CODE_GENERATOR_H_
