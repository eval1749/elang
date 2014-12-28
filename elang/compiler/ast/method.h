// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_method_h)
#define INCLUDE_elang_compiler_ast_method_h

#include <vector>

#include "elang/compiler/ast/namespace_member.h"
#include "elang/compiler/ast/type_and_name.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {
namespace ast {

class Class;
class MethodMember;
class NodeFactory;

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public NamespaceMember {
  DECLARE_CASTABLE_CLASS(Method, NamespaceMember);

  friend class NodeFactory;

 public:
  typedef std::vector<TypeAndName> Parameters;

  ~Method() final;

  const Parameters& parameters() const { return parameters_; }
  Expression* return_type() const { return return_type_; }
  const std::vector<Token*>& type_parameters() {  return type_parameters_; }

 private:
  Method(NamespaceBody* namespace_body, Modifiers modifies,
                 Expression* return_type, Token* name,
                 const std::vector<Token*>& type_parameters,
                 const Parameters& parameters);

  // Node
  void Accept(Visitor* visitor) override;

  const Parameters parameters_;
  Expression* return_type_;
  const std::vector<Token*> type_parameters_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // !defined(INCLUDE_elang_compiler_ast_method_h)

