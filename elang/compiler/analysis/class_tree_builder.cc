// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "elang/compiler/analysis/class_tree_builder.h"

#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/analysis/name_resolver_editor.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/modifiers.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"

namespace elang {
namespace compiler {

namespace {

bool IsFixed(sm::Semantic* semantic) {
  if (auto const clazz = semantic->as<sm::Class>())
    return clazz->has_base();
  return true;
}

bool IsNamespaceOrType(sm::Semantic* semantic) {
  return semantic->is<sm::Type>() || semantic->is<sm::Namespace>();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// ClassTreeBuilder::ClassData
//
class ClassTreeBuilder::ClassData final : public ZoneAllocated {
 public:
  ClassData(Zone* zone, sm::Class* clazz);
  ~ClassData() = delete;

  const ZoneVector<ast::Class*>& partial_classes() const {
    return partial_classes_;
  }

  void AddClass(ast::Class* ast_class);

 private:
  sm::Class* const class_;
  ZoneVector<ast::Class*> partial_classes_;

  DISALLOW_COPY_AND_ASSIGN(ClassData);
};

ClassTreeBuilder::ClassData::ClassData(Zone* zone, sm::Class* clazz)
    : class_(clazz), partial_classes_(zone) {
}

void ClassTreeBuilder::ClassData::AddClass(ast::Class* ast_class) {
  DCHECK(std::find(partial_classes_.begin(), partial_classes_.end(),
                   ast_class) == partial_classes_.end());
  partial_classes_.push_back(ast_class);
}

//////////////////////////////////////////////////////////////////////
//
// ClassTreeBuilder
//
ClassTreeBuilder::ClassTreeBuilder(NameResolver* resolver, sm::Editor* editor)
    : CompilationSessionUser(resolver->session()),
      resolver_editor_(new NameResolverEditor(resolver)),
      semantic_editor_(editor) {
}

ClassTreeBuilder::~ClassTreeBuilder() {
}

void ClassTreeBuilder::AnalyzeClass(ast::Class* node) {
  auto const clazz = SemanticOf(node)->as<sm::Class>();
  DCHECK(!IsFixed(clazz)) << clazz;
  auto const outer = node->parent()->as<ast::ContainerNode>();
  auto const class_data = ClassDataFor(clazz);
  class_data->AddClass(node);
  if (auto const outer_class = clazz->outer()->as<sm::Class>())
    MarkDepdency(clazz, outer_class);
  for (auto const base_class_name : node->base_class_names()) {
    auto const present = Resolve(node, base_class_name, outer);
    if (!present) {
      unresolved_names_.insert(base_class_name);
      continue;
    }
    auto const base_class = present->as<sm::Class>();
    if (!base_class)
      continue;
    if (clazz == base_class)
      Error(ErrorCode::ClassTreeBaseClassSelf, node, base_class_name);
    MarkDepdency(clazz, base_class);
  }
  auto const default_base_class = DefaultBaseClassFor(clazz);
  if (!default_base_class || clazz == default_base_class)
    return;
  MarkDepdency(clazz, default_base_class);
}

ClassTreeBuilder::ClassData* ClassTreeBuilder::ClassDataFor(sm::Class* clazz) {
  auto const it = class_data_map_.find(clazz);
  if (it != class_data_map_.end())
    return it->second;
  auto const class_data = new (zone()) ClassData(zone(), clazz);
  class_data_map_.insert(std::make_pair(clazz, class_data));
  return class_data;
}

sm::Class* ClassTreeBuilder::DefaultBaseClassFor(sm::Class* clazz) {
  if (clazz->is_class())
    return PredefinedTypeOf(PredefinedName::Object)->as<sm::Class>();
  if (clazz->is_struct())
    return PredefinedTypeOf(PredefinedName::ValueType)->as<sm::Class>();
  if (clazz->is_interface())
    return nullptr;
  NOTREACHED() << clazz;
  return nullptr;
}

void ClassTreeBuilder::FindInClass(Token* name,
                                   sm::Class* clazz,
                                   std::unordered_set<sm::Semantic*>* founds) {
  DCHECK(IsFixed(clazz)) << clazz;
  for (auto const base_class : clazz->direct_base_classes()) {
    DCHECK(IsFixed(base_class)) << base_class;
    if (auto const present = base_class->FindMember(name)) {
      founds->insert(present);
      continue;
    }
    FindInClass(name, base_class, founds);
  }
}

void ClassTreeBuilder::FixClass(sm::Class* clazz) {
  if (IsFixed(clazz))
    return;
  DCHECK(IsFixed(clazz->outer())) << clazz->outer();
  std::unordered_set<sm::Class*> presents;
  std::vector<sm::Class*> base_class_candidates;
  std::vector<sm::Class*> interfaces;
  auto const class_data = ClassDataFor(clazz);
  for (auto const ast_class : class_data->partial_classes()) {
    std::unordered_set<sm::Class*> direct_presents;
    auto position = 0;
    for (auto const base_class_name : ast_class->base_class_names()) {
      ++position;
      auto const base_class =
          ValidateBaseClass(ast_class, clazz, position, base_class_name);
      if (!base_class)
        continue;
      DCHECK(IsFixed(base_class)) << base_class_name;
      if (direct_presents.count(base_class)) {
        Error(ErrorCode::ClassTreeBaseClassDuplicate, base_class_name);
        continue;
      }
      direct_presents.insert(base_class);
      if (presents.count(base_class))
        continue;
      presents.insert(base_class);
      if (base_class->is_interface()) {
        interfaces.push_back(base_class);
        continue;
      }
      base_class_candidates.push_back(base_class);
    }
  }

  if (clazz->is_interface()) {
    DCHECK(base_class_candidates.empty());
    std::vector<sm::Class*> base_class_list(interfaces.begin(),
                                            interfaces.end());
    semantic_editor_->FixClassBase(clazz, base_class_list);
    return;
  }

  if (base_class_candidates.size() >= 2) {
    for (auto const base_class : base_class_candidates)
      Error(ErrorCode::ClassTreeBaseClassConflict, base_class->name());
    return;
  }

  std::vector<sm::Class*> base_class_list;
  if (base_class_candidates.empty()) {
    if (auto const default_base_class = DefaultBaseClassFor(clazz)) {
      if (clazz != default_base_class)
        base_class_list.push_back(default_base_class);
    }
  } else if (base_class_candidates.size() == 1) {
    auto const base_class = base_class_candidates.front();
    DCHECK(!base_class->is_interface());
    DCHECK_EQ(clazz->is_class(), base_class->is_class());
    base_class_list.push_back(base_class);
  }

  for (auto const interface : interfaces)
    base_class_list.push_back(interface);

  semantic_editor_->FixClassBase(clazz, base_class_list);
}

void ClassTreeBuilder::MarkDepdency(sm::Class* clazz, sm::Class* using_class) {
  if (IsFixed(clazz) && IsFixed(using_class))
    return;
  if (dependency_graph_.HasEdge(clazz, using_class))
    return;
  dependency_graph_.AddEdge(clazz, using_class);
}

sm::Semantic* ClassTreeBuilder::Resolve(ast::Node* client,
                                        ast::Node* node,
                                        ast::Node* context_node) {
  if (auto const ref = node->as<ast::MemberAccess>())
    return ResolveMemberAccess(client, ref, context_node);
  if (auto const ref = node->as<ast::NameReference>())
    return ResolveNameReference(client, ref, context_node);
  if (auto const ref = node->as<ast::TypeMemberAccess>())
    return Resolve(client, ref->reference(), context_node);
  if (auto const ref = node->as<ast::TypeNameReference>())
    return Resolve(client, ref->reference(), context_node);
  NOTREACHED() << node;
  return nullptr;
}

// Reference of |ast::Alias| are resolved in enclosing container of
// enclosing container of |ast::Alias|, e.g. looking into namespace N1 in
// below example:
//
//  namespace N1 {
//    namespace N2 {
//      using R1 = A;
//      class A {}
//      class B : R1 {}  // base_class_of(B) == N1.A
//    }
//    class A {}
//  }
sm::Semantic* ClassTreeBuilder::ResolveAlias(ast::Alias* alias) {
  auto const present = Resolve(alias, alias->reference(), alias->parent());
  if (!present)
    return nullptr;
  if (!IsNamespaceOrType(present)) {
    Error(ErrorCode::ClassTreeAliasNeitherNamespaceNorType, alias);
    return nullptr;
  }
  return present;
}

sm::Semantic* ClassTreeBuilder::ResolveMemberAccess(ast::Node* client,
                                                    ast::MemberAccess* node,
                                                    ast::Node* context_node) {
  auto const container = Resolve(client, node->container(), context_node);
  if (!container)
    return container;
  if (auto const member = container->FindMember(node->name()))
    return member;
  if (!IsFixed(container))
    return container;
  auto const clazz = container->as<sm::Class>();
  if (!clazz) {
    Error(ErrorCode::ClassTreeNameNotFound, node);
    return nullptr;
  }
  std::unordered_set<sm::Semantic*> founds;
  FindInClass(node->name(), clazz, &founds);
  if (founds.empty()) {
    Error(ErrorCode::ClassTreeNameNotFound, node);
    return nullptr;
  }
  if (founds.size() >= 2) {
    for (auto const found : founds)
      Error(ErrorCode::ClassTreeNameAmbiguous, node, found->name());
    return nullptr;
  }
  return *founds.begin();
}

sm::Semantic* ClassTreeBuilder::ResolveNameReference(ast::Node* client,
                                                     ast::NameReference* node,
                                                     ast::Node* context_node) {
  auto const ignoring_container =
      client->is<ast::Alias>() || client->is<ast::Import>() ? context_node
                                                            : nullptr;
  auto const name = node->name();
  for (auto runner = context_node; runner;
       runner = runner->parent()->as<ast::ContainerNode>()) {
    auto const outer = SemanticOf(runner);
    if (!IsFixed(outer))
      return outer;
    std::unordered_set<sm::Semantic*> founds;
    if (runner == ignoring_container) {
      DCHECK(outer->is<sm::Namespace>());
      if (auto const present = outer->FindMember(name)->as<sm::Namespace>())
        founds.insert(present);
    } else if (auto const ns_body = runner->as<ast::NamespaceBody>()) {
      DCHECK(outer->is<sm::Namespace>());
      if (auto const present = outer->FindMember(name))
        founds.insert(present);
      if (auto const alias = ns_body->FindAlias(name)) {
        unused_aliases_.erase(alias);
        auto const resolved = ResolveAlias(alias);
        if (!resolved)
          return nullptr;
        founds.insert(resolved);
      }
      if (founds.empty())
        resolver_editor_->FindWithImports(name, ns_body, &founds);
    } else if (auto const clazz = outer->as<sm::Class>()) {
      if (auto const present = outer->FindMember(name))
        return present;
      FindInClass(name, clazz, &founds);
    } else if (auto const present = outer->FindMember(name)) {
      return present;
    }
    if (founds.size() == 1)
      return *founds.begin();
    if (founds.size() >= 2) {
      Error(ErrorCode::ClassTreeNameAmbiguous, node);
      return nullptr;
    }
  }
  Error(ErrorCode::ClassTreeNameNotFound, node);
  return nullptr;
}

// The entry point of |ClassTreeBuilder|.
void ClassTreeBuilder::Run() {
  session()->Apply(this);
  auto const all_classes = dependency_graph_.GetAllVertices();
  std::vector<sm::Class*> leaf_classes;
  for (auto const clazz : all_classes) {
    if (dependency_graph_.HasInEdge(clazz))
      continue;
    leaf_classes.push_back(clazz);
  }
  std::unordered_set<sm::Class*> processed;
  for (auto const leaf_class : leaf_classes) {
    for (auto const clazz : dependency_graph_.PostOrderListOf(leaf_class)) {
      FixClass(clazz);
      processed.insert(clazz);
    }
  }
  // Check unused aliases resolve-able
  for (auto const alias : unused_aliases_)
    ResolveAlias(alias);
  // Report cycle classes
  std::set<std::pair<sm::Class*, sm::Class*>> cycles;
  for (auto const clazz : all_classes) {
    if (processed.count(clazz))
      continue;
    for (auto const using_class : dependency_graph_.GetOutEdges(clazz)) {
      if (IsFixed(using_class))
        continue;
      auto const key = std::make_pair(using_class, clazz);
      if (cycles.count(key))
        continue;
      cycles.insert(key);
      if (clazz == using_class)
        continue;
      Error(ErrorCode::ClassTreeClassCycle, clazz->name(), using_class->name());
    }
  }
}

sm::Semantic* ClassTreeBuilder::SemanticOf(ast::Node* node) const {
  return session()->analysis()->SemanticOf(node);
}

sm::Class* ClassTreeBuilder::ValidateBaseClass(ast::Class* ast_class,
                                               sm::Class* clazz,
                                               int position,
                                               ast::Node* base_class_name) {
  if (unresolved_names_.count(base_class_name))
    return nullptr;
  auto const present = Resolve(ast_class, base_class_name, ast_class->parent());
  if (!present)
    return nullptr;
  // TODO(eval1749) Check |base_class| isn't |final|.
  // TODO(eval1749) We should check accessibility of |base_class|.
  auto const base_class = present->as<sm::Class>();
  if (base_class) {
    auto const outer = clazz->outer();
    if (base_class == outer || outer->IsDescendantOf(base_class)) {
      Error(ErrorCode::ClassTreeBaseClassContaining, base_class_name,
            clazz->name());
      return nullptr;
    }
    if (base_class->is_interface())
      return base_class;
  }

  if (clazz->is_interface() || position >= 2) {
    Error(ErrorCode::ClassTreeBaseClassNotInterface, base_class_name);
    return nullptr;
  }

  if (clazz->is_class()) {
    if (base_class && base_class->is_class())
      return base_class;
    Error(ErrorCode::ClassTreeBaseClassNeitherClassNorInterface,
          base_class_name);
    return nullptr;
  }

  if (clazz->is_struct()) {
    if (base_class && base_class->is_struct())
      return base_class;
    Error(ErrorCode::ClassTreeBaseClassNeitherStructNorInterface,
          base_class_name);
    return nullptr;
  }

  NOTREACHED() << clazz << " " << base_class_name;
  return nullptr;
}

// ast::Visitor member function implementations
void ClassTreeBuilder::VisitAlias(ast::Alias* node) {
  unused_aliases_.insert(node);
}

void ClassTreeBuilder::VisitImport(ast::Import* node) {
  auto const imported = Resolve(node, node->reference(), node->parent());
  if (!imported)
    return resolver_editor_->RegisterImport(
        node, static_cast<sm::Namespace*>(nullptr));
  if (auto const ns = imported->as<sm::Namespace>())
    return resolver_editor_->RegisterImport(node, ns);
  Error(ErrorCode::ClassTreeImportNotNamespace, node->reference());
  resolver_editor_->RegisterImport(node, static_cast<sm::Namespace*>(nullptr));
}

void ClassTreeBuilder::VisitClass(ast::Class* node) {
  AnalyzeClass(node);
  ast::Visitor::VisitClass(node);
}

}  // namespace compiler
}  // namespace elang
