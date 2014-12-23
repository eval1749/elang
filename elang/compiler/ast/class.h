// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_ast_class_h)
#define elang_compiler_ast_class_h

#include <vector>

#include "elang/compiler/ast/namespace.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  friend class NodeFactory;

  private: std::vector<QualifiedName> base_class_names_;

  private: Class(Namespace* outer, const Token& keyword,
                 const Token& simple_name);
  public: ~Class() final;

  public: void AddBaseClassName(const QualifiedName& class_name);

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_ast_class_h)

