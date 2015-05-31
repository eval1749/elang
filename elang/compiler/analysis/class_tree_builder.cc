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

  const ZoneVector<ast::ClassBody*>& class_bodies() const {
    return class_bodies_;
  }

  void AddClassBody(ast::ClassBody* class_body);

 private:
  sm::Class* const class_;
  ZoneVector<ast::ClassBody*> class_bodies_;

  DISALLOW_COPY_AND_ASSIGN(ClassData);
};

ClassTreeBuilder::ClassData::ClassData(Zone* zone, sm::Class* clazz)
    : class_(clazz), class_bodies_(zone) {
}

void ClassTreeBuilder::ClassData::AddClassBody(ast::ClassBody* class_body) {
  DCHECK(std::find(class_bodies_.begin(), class_bodies_.end(), class_body) ==
         class_bodies_.end());
  class_bodies_.push_back(class_body);
}

//////////////////////////////////////////////////////////////////////
//
// ClassTreeBuilder
//
ClassTreeBuilder::ClassTreeBuilder(CompilationSession* session,
                                   sm::Editor* editor)
    : CompilationSessionUser(session), semantic_editor_(editor) {
}

ClassTreeBuilder::~ClassTreeBuilder() {
}

void ClassTreeBuilder::AnalyzeClassBody(ast::ClassBody* node) {
  auto const clazz = SemanticOf(node)->as<sm::Class>();
  if (auto const ns = node->parent()->as<ast::NamespaceBody>()) {
    if (ns->loaded_) {
      DCHECK(IsFixed(clazz)) << clazz;
      return;
    }
  }
  DCHECK(!IsFixed(clazz)) << clazz;
  auto const outer = node->parent()->as<ast::ContainerNode>();
  auto const class_data = ClassDataFor(clazz);
  class_data->AddClassBody(node);
  if (auto const outer_class = clazz->outer()->as<sm::Class>())
    MarkDepdency(clazz, outer_class);
  for (auto const base_class_name : node->base_class_names()) {
    auto const present = Resolve(base_class_name, outer);
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

void ClassTreeBuilder::FindWithImports(
    Token* name,
    ast::NamespaceBody* ns_body,
    std::unordered_set<sm::Semantic*>* founds) {
  for (auto const pair : ns_body->imports()) {
    auto const import = pair.second;
    auto const it = import_map_.find(import);
    DCHECK(it != import_map_.end());
    auto const imported_ns = it->second;
    if (!imported_ns)
      continue;
    auto const present = imported_ns->FindMember(name);
    if (!present)
      continue;
    if (present->is<sm::Namespace>()) {
      // Import directive doesn't import nested namespace.
      // Example:
      //   namespace N1.N2 { class A {} }
      //   namespace N3 { using N1; class B : N2.A {} }
      // Reference |N2.A| is undefined, since |using N1| doesn't import
      // nested namespace N1.N2.
      continue;
    }
    founds->insert(present);
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
  for (auto const class_body : class_data->class_bodies()) {
    auto const outer = class_body->parent();
    std::unordered_set<sm::Class*> direct_presents;
    auto position = 0;
    for (auto const base_class_name : class_body->base_class_names()) {
      ++position;
      auto const base_class =
          ValidateBaseClass(clazz, position, base_class_name, outer);
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

sm::Semantic* ClassTreeBuilder::Resolve(ast::Node* node,
                                        ast::Node* context_node) {
  if (auto const ref = node->as<ast::MemberAccess>())
    return ResolveMemberAccess(ref, context_node);
  if (auto const ref = node->as<ast::NameReference>())
    return ResolveNameReference(ref, context_node);
  if (auto const ref = node->as<ast::TypeMemberAccess>())
    return Resolve(ref->reference(), context_node);
  if (auto const ref = node->as<ast::TypeNameReference>())
    return Resolve(ref->reference(), context_node);
  NOTREACHED() << node;
  return nullptr;
}

sm::Semantic* ClassTreeBuilder::ResolveMemberAccess(ast::MemberAccess* node,
                                                    ast::Node* context_node) {
  auto const container = Resolve(node->container(), context_node);
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

sm::Semantic* ClassTreeBuilder::ResolveNameReference(ast::NameReference* node,
                                                     ast::Node* context_node) {
  auto const name = node->name();
  for (auto runner = context_node; runner;
       runner = runner->parent()->as<ast::ContainerNode>()) {
    auto const outer = SemanticOf(runner);
    if (!IsFixed(outer))
      return outer;
    std::unordered_set<sm::Semantic*> founds;
    if (auto const ns_body = runner->as<ast::NamespaceBody>()) {
      if (auto const present = outer->FindMember(name))
        founds.insert(present);
      if (auto const alias = ns_body->FindMember(name)->as<ast::Alias>()) {
        unused_aliases_.erase(alias);
        founds.insert(Resolve(alias->reference(), ns_body->parent()));
      }
      if (founds.empty())
        FindWithImports(name, ns_body, &founds);
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

void ClassTreeBuilder::Run() {
  Traverse(session()->global_namespace_body());
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
  for (auto const alias : unused_aliases_) {
    Error(ErrorCode::ClassTreeAliasNotUsed, alias->name());
    auto const result = Resolve(alias->reference(), alias->parent()->parent());
    if (!result || IsNamespaceOrType(result))
      continue;
    Error(ErrorCode::ClassTreeAliasNeitherNamespaceNorType, alias->reference());
  }
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

sm::Class* ClassTreeBuilder::ValidateBaseClass(sm::Class* clazz,
                                               int position,
                                               ast::Node* base_class_name,
                                               ast::Node* container) {
  if (unresolved_names_.count(base_class_name))
    return nullptr;
  auto const present = Resolve(base_class_name, container);
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
  DCHECK(!import_map_.count(node)) << node;
  auto const context = node->parent()->parent();
  auto const imported = Resolve(node->reference(), context);
  if (!imported) {
    import_map_.insert(std::make_pair(node, nullptr));
    return;
  }
  if (auto const ns = imported->as<sm::Namespace>()) {
    import_map_.insert(std::make_pair(node, ns));
    return;
  }
  Error(ErrorCode::ClassTreeImportNotNamespace, node->reference());
  import_map_.insert(std::make_pair(node, nullptr));
}

void ClassTreeBuilder::VisitClassBody(ast::ClassBody* node) {
  AnalyzeClassBody(node);
  ast::Visitor::VisitClassBody(node);
}

}  // namespace compiler
}  // namespace elang
