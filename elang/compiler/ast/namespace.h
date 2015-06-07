// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_AST_NAMESPACE_H_
#define ELANG_COMPILER_AST_NAMESPACE_H_

#include "elang/compiler/ast/container_node.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Alias
//
class Alias final : public SimpleNode<NamedNode, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(Alias, NamedNode);

 public:
  Expression* reference() const;

 private:
  Alias(NamespaceBody* namespace_body,
        Token* keyword,
        Token* alias_name,
        Expression* reference);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
#endif

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

//////////////////////////////////////////////////////////////////////
//
// Import
// Note: Instances of |Import| class aren't appeared in |ContainerNode|'s
// named map.
//
class Import final : public SimpleNode<NamedNode, 1> {
  DECLARE_CONCRETE_AST_NODE_CLASS(Import, NamedNode);

 public:
  Expression* reference() const;

 private:
  Import(NamespaceBody* namespace_body, Token* keyword, Expression* reference);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
#endif

  DISALLOW_COPY_AND_ASSIGN(Import);
};

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
class NamespaceBody final : public ContainerNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(NamespaceBody, ContainerNode);

 public:
  typedef ZoneUnorderedMap<AtomicString*, Import*> ImportMap;

  const ImportMap& imports() const { return import_map_; }
  NamespaceBody* outer() const;

  void AddImport(Import* import);

  // Returns |Alias| named by |name| in this namespace or null if not found.
  Alias* FindAlias(Token* name) const;

  // Returns |Import| named by |name| in this namespace or null if not found.
  Import* FindImport(Token* name) const;

 private:
  NamespaceBody(Zone* zone, NamespaceBody* outer, Token* keyword, Token* name);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
#endif

  ImportMap import_map_;

  DISALLOW_COPY_AND_ASSIGN(NamespaceBody);
};

}  // namespace ast
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_AST_NAMESPACE_H_
