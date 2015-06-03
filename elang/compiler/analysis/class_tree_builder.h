// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_CLASS_TREE_BUILDER_H_
#define ELANG_COMPILER_ANALYSIS_CLASS_TREE_BUILDER_H_

#include <memory>
#include <unordered_map>
#include <unordered_set>

#include "elang/base/simple_directed_graph.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {

namespace sm {
class Class;
class Editor;
class Namespace;
class Semantic;
}
class NameResolver;
class NameResolverEditor;

//////////////////////////////////////////////////////////////////////
//
// ClassTreeBuilder
//
class ClassTreeBuilder final : public CompilationSessionUser,
                               public ast::Visitor,
                               public ZoneOwner {
 public:
  ClassTreeBuilder(NameResolver* resolver_editor, sm::Editor* editor);
  ~ClassTreeBuilder() final;

  // The Entry Point of |ClassTreeBuilder|
  void Run();

 private:
  class ClassData;

  void AnalyzeClass(ast::Class* node);
  ClassData* ClassDataFor(sm::Class* clazz);
  sm::Class* DefaultBaseClassFor(sm::Class* clazz);
  void FindInClass(Token* name,
                   sm::Class* clazz,
                   std::unordered_set<sm::Semantic*>* founds);
  void FixClass(sm::Class* clazz);
  void MarkDepdency(sm::Class* clazz, sm::Class* using_class);
  sm::Semantic* Resolve(ast::Node* client,
                        ast::Node* node,
                        ast::Node* context_node);
  sm::Semantic* ResolveAlias(ast::Alias* alias);
  sm::Semantic* ResolveMemberAccess(ast::Node* client,
                                    ast::MemberAccess* node,
                                    ast::Node* context_node);
  sm::Semantic* ResolveNameReference(ast::Node* client,
                                     ast::NameReference* node,
                                     ast::Node* context_node);
  sm::Semantic* SemanticOf(ast::Node* node) const;
  sm::Class* ValidateBaseClass(ast::Class* ast_class,
                               sm::Class* clazz,
                               int position,
                               ast::Node* base_class_name);

  // ast::Visitor
  void VisitAlias(ast::Alias* node) final;
  void VisitImport(ast::Import* node) final;
  void VisitClass(ast::Class* node) final;

  std::unordered_map<sm::Class*, ClassData*> class_data_map_;
  SimpleDirectedGraph<sm::Class*> dependency_graph_;
  std::unique_ptr<NameResolverEditor> const resolver_editor_;
  sm::Editor* const semantic_editor_;
  std::unordered_set<ast::Node*> unresolved_names_;
  std::unordered_set<ast::Alias*> unused_aliases_;

  DISALLOW_COPY_AND_ASSIGN(ClassTreeBuilder);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_CLASS_TREE_BUILDER_H_
