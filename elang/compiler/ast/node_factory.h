// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_ast_node_factory_h)
#define INCLUDE_elang_compiler_ast_node_factory_h

#include <memory>
#include <vector>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/source_code_range.h"

namespace elang {
namespace compiler {
class QualifiedName;
class Token;

namespace ast {
class Class;
class Enum;
class EnumMember;
class Expression;
class Namespace;
class Node;

//////////////////////////////////////////////////////////////////////
//
// NodeFactory
//
class NodeFactory final {
  private: std::vector<std::unique_ptr<Node>> nodes_;

  public: NodeFactory();
  public: ~NodeFactory();

  public: Class* NewClass(Namespace* outer, const Token& keyword,
                          const Token& simple_name);
  public: Enum* NewEnum(Namespace* outer, const Token& keyword,
                        const Token& simple_name);
  public: EnumMember* NewEnumMember(Enum* owner, const Token& simple_name,
                                    Expression* expression);
  public: Namespace* NewNamespace(Namespace* outer, const Token& keyword,
                                  QualifiedName&& simple_name);

  DISALLOW_COPY_AND_ASSIGN(NodeFactory);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_ast_node_factory_h)

