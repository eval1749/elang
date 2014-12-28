// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_class_h)
#define INCLUDE_elang_compiler_ast_class_h

#include <vector>

#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/qualified_name.h"

namespace elang {
namespace compiler {
namespace ast {

class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Namespace {
  DECLARE_CASTABLE_CLASS(Class, Namespace);

  friend class NodeFactory;

 public:
  ~Class() final;

  const std::vector<Class*>& base_classes() const;
  const std::vector<QualifiedName>& base_class_names() const {
    return base_class_names_;
  }
  bool is_fixed() const {return is_fixed_; }

  void AddBaseClassName(const QualifiedName& class_name);

  void BindBaseClasses(const std::vector<Class*>& base_classes);

  // NamespaceMember
  Namespace* ToNamespace() final;

 private:
  Class(NamespaceBody* namespace_body, Modifiers modifiers,
                 Token* keyword, Token* name);
  // Node
  void Accept(Visitor* visitor) override;

  std::vector<Class*> base_classes_;
  std::vector<QualifiedName> base_class_names_;
  bool is_fixed_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_class_h)

