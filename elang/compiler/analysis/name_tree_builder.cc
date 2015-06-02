// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/analysis/name_tree_builder.h"

#include "base/logging.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

NameTreeBuilder::NameTreeBuilder(CompilationSession* session,
                                 AnalysisEditor* editor)
    : CompilationSessionUser(session), editor_(editor) {
}

NameTreeBuilder::~NameTreeBuilder() {
}

sm::Factory* NameTreeBuilder::factory() const {
  return session()->semantic_factory();
}

sm::Class* NameTreeBuilder::NewClass(ast::ClassBody* node) {
  auto const outer = SemanticOf(node->parent());
  if (node->is_class())
    return factory()->NewClass(outer, node->modifiers(), node->name());
  if (node->is_interface())
    return factory()->NewInterface(outer, node->modifiers(), node->name());
  if (node->is_struct())
    return factory()->NewStruct(outer, node->modifiers(), node->name());
  NOTREACHED() << node;
  return nullptr;
}

void NameTreeBuilder::ProcessNamespaceBody(ast::NamespaceBody* node) {
  if (!node->parent()) {
    editor_->SetSemanticOf(node, factory()->global_namespace());
    return;
  }
  // Check duplicated aliases
  {
    std::unordered_map<AtomicString*, ast::Alias*> aliases_;
    for (auto const member : node->members()) {
      auto const alias = member->as<ast::Alias>();
      if (!alias)
        continue;
      auto const key = alias->name()->atomic_string();
      auto const it = aliases_.find(key);
      if (it != aliases_.end()) {
        Error(ErrorCode::NameTreeAliasDuplicate, alias->name(),
              it->second->name());
        continue;
      }
      aliases_.insert(std::make_pair(key, alias));
    }
  }
  auto const outer = SemanticOf(node->outer())->as<sm::Namespace>();
  if (!outer->is<sm::Namespace>())
    return;
  if (auto const present = outer->FindMember(node->name())) {
    if (!present->is<sm::Namespace>()) {
      Error(ErrorCode::NameTreeNamespaceConflict, node->name(),
            present->name());
    }
    editor_->SetSemanticOf(node, present);
    return;
  }
  auto const ns = factory()->NewNamespace(outer, node->name());
  editor_->SetSemanticOf(node, ns);
  editor_->SetSemanticOf(node->owner(), ns);
}

void NameTreeBuilder::Run() {
  session()->Apply(this);
  for (auto const alias : aliases_) {
    auto const outer = SemanticOf(alias->parent());
    auto const present = outer->FindMember(alias->name());
    if (!present)
      continue;
    Error(ErrorCode::NameTreeAliasConflict, alias->name(), present->name());
  }
}

sm::Semantic* NameTreeBuilder::SemanticOf(ast::Node* node) const {
  return editor_->SemanticOf(node);
}

// NameTreeBuilder - ast::Visitor
void NameTreeBuilder::VisitAlias(ast::Alias* node) {
  aliases_.push_back(node);
}

void NameTreeBuilder::VisitClassBody(ast::ClassBody* node) {
  auto const outer = SemanticOf(node->parent());
  auto const present = outer->FindMember(node->name());
  if (!present) {
    editor_->SetSemanticOf(node, NewClass(node));
    ast::Visitor::VisitClassBody(node);
    return;
  }
  if (auto const present_class = present->as<sm::Class>()) {
    if (!present_class->has_base() && node->IsPartial() &&
        present_class->IsPartial()) {
      editor_->SetSemanticOf(node, present_class);
      return ast::Visitor::VisitClassBody(node);
    }
    return Error(ErrorCode::NameTreeClassDuplicate, node, present->name());
  }
  Error(ErrorCode::NameTreeClassConflict, node, present->name());
}

void NameTreeBuilder::VisitConst(ast::Const* node) {
  auto const owner =
      SemanticOf(node->parent()->as<ast::ClassBody>())->as<sm::Class>();
  auto const present = owner->FindMember(node->name());
  if (!present) {
    editor_->SetSemanticOf(node, factory()->NewConst(owner, node->name()));
    return;
  }
  if (present->is<sm::Const>()) {
    return Error(ErrorCode::NameTreeConstDuplicate, node, present->name());
  }
  Error(ErrorCode::NameTreeConstConflict, node, present->name());
}

void NameTreeBuilder::VisitEnum(ast::Enum* node) {
  auto const outer = SemanticOf(node->parent());
  auto const present = outer->FindMember(node->name());
  if (present) {
    if (present->is<sm::Enum>())
      return Error(ErrorCode::NameTreeEnumDuplicate, node, present->name());
    return Error(ErrorCode::NameTreeEnumConflict, node, present->name());
  }

  auto const enum_type = factory()->NewEnum(outer, node->name());
  for (auto const member : node->members()) {
    editor_->SetSemanticOf(member,
                           factory()->NewEnumMember(enum_type, member->name()));
  }
  editor_->SetSemanticOf(node, enum_type);
}

void NameTreeBuilder::VisitField(ast::Field* node) {
  auto const owner =
      SemanticOf(node->parent()->as<ast::ClassBody>())->as<sm::Class>();
  auto const present = owner->FindMember(node->name());
  if (!present) {
    editor_->SetSemanticOf(node, factory()->NewField(owner, node->name()));
    return;
  }
  if (present->is<sm::Field>())
    return Error(ErrorCode::NameTreeFieldDuplicate, node, present->name());
  Error(ErrorCode::NameTreeFieldConflict, node, present->name());
}

void NameTreeBuilder::VisitMethod(ast::Method* node) {
  auto const owner =
      SemanticOf(node->parent()->as<ast::ClassBody>())->as<sm::Class>();
  auto const present = owner->FindMember(node->name());
  if (!present) {
    factory()->NewMethodGroup(owner, node->name());
    return;
  }
  if (present->is<sm::MethodGroup>())
    return;
  Error(ErrorCode::NameTreeMethodConflict, node->name(), present->name());
}

void NameTreeBuilder::VisitNamespaceBody(ast::NamespaceBody* node) {
  ProcessNamespaceBody(node);
  ast::Visitor::VisitNamespaceBody(node);
}

}  // namespace compiler
}  // namespace elang
