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
class Alias final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Alias, NamedNode);

 public:
  Expression* reference() const { return reference_; }

 private:
  Alias(NamespaceBody* namespace_body,
        Token* keyword,
        Token* alias_name,
        Expression* reference);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  Expression* const reference_;

  DISALLOW_COPY_AND_ASSIGN(Alias);
};

//////////////////////////////////////////////////////////////////////
//
// Import
// Note: Instances of |Import| class aren't appeared in |ContainerNode|'s
// named map.
//
class Import final : public NamedNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Import, NamedNode);

 public:
  Expression* reference() const { return reference_; }

 private:
  Import(NamespaceBody* namespace_body, Token* keyword, Expression* reference);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
#endif

  Expression* const reference_;

  DISALLOW_COPY_AND_ASSIGN(Import);
};

//////////////////////////////////////////////////////////////////////
//
// Namespace
//
class Namespace final : public NamespaceNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(Namespace, NamespaceNode);

 private:
  Namespace(Zone* zone, Namespace* outer, Token* keyword, Token* name);

#if _DEBUG
  // Node
  bool CanBeMemberOf(ContainerNode* container) const final;
  // NamedNode
  bool CanBeNamedMemberOf(ContainerNode* container) const final;
#endif

  DISALLOW_COPY_AND_ASSIGN(Namespace);
};

//////////////////////////////////////////////////////////////////////
//
// NamespaceBody
//
class NamespaceBody final : public BodyNode {
  DECLARE_CONCRETE_AST_NODE_CLASS(NamespaceBody, BodyNode);

 public:
  typedef ZoneUnorderedMap<AtomicString*, Import*> ImportMap;

  const ImportMap& imports() const { return import_map_; }
  NamespaceBody* outer() const;
  Namespace* owner() const;

  void AddImport(Import* import);

  // Returns |Alias| named by |name| in this namespace or null if not found.
  Alias* FindAlias(Token* name) const;

  // Returns |Import| named by |name| in this namespace or null if not found.
  Import* FindImport(Token* name) const;

  // TODO(eval1749) We should use Creator::Parser, Loader, etc.
  bool loaded_;

 private:
  NamespaceBody(Zone* zone, NamespaceBody* outer, Namespace* owner);

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
