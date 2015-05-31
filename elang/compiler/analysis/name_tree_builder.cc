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

sm::Class* NameTreeBuilder::NewClass(ast::ClassBody* node) {
  auto const outer = SemanticOf(node->parent());
  if (node->owner()->is_class()) {
    return session()->semantic_factory()->NewClass(outer, node->modifiers(),
                                                   node->name(), node->owner());
  }
  if (node->owner()->is_interface()) {
    return session()->semantic_factory()->NewInterface(outer, node->modifiers(),
                                                       node->name());
  }
  if (node->owner()->is_struct()) {
    return session()->semantic_factory()->NewStruct(outer, node->modifiers(),
                                                    node->name());
  }
  NOTREACHED() << node;
  return nullptr;
}

void NameTreeBuilder::ProcessNamespaceBody(ast::NamespaceBody* node) {
  if (node->loaded_)
    return;
  if (node->owner() == session()->global_namespace()) {
    editor_->SetSemanticOf(node,
                           session()->semantic_factory()->global_namespace());
    return;
  }
  auto const outer = SemanticOf(node->outer())->as<sm::Namespace>();
  DCHECK(outer->is<sm::Namespace>()) << outer;
  if (auto const present = outer->FindMember(node->name())) {
    DCHECK(present->is<sm::Namespace>()) << present;
    editor_->SetSemanticOf(node, present);
    return;
  }
  auto const ns =
      session()->semantic_factory()->NewNamespace(outer, node->name());
  editor_->SetSemanticOf(node, ns);
  editor_->SetSemanticOf(node->owner(), ns);
}

void NameTreeBuilder::Run() {
  Traverse(session()->global_namespace_body());
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
  if (auto const ns = node->parent()->as<ast::NamespaceBody>()) {
    if (ns->loaded_)
      return;
  }
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
    editor_->SetSemanticOf(
        node, session()->semantic_factory()->NewConst(owner, node->name()));
    return;
  }
  if (present->is<sm::Const>()) {
    return Error(ErrorCode::NameTreeConstDuplicate, node, present->name());
  }
  Error(ErrorCode::NameTreeConstConflict, node, present->name());
}

void NameTreeBuilder::VisitEnum(ast::Enum* node) {
  if (auto const ns = node->parent()->as<ast::NamespaceBody>()) {
    if (ns->loaded_)
      return;
  }
  auto const outer = SemanticOf(node->parent());
  auto const present = outer->FindMember(node->name());
  if (!present) {
    editor_->SetSemanticOf(
        node, session()->semantic_factory()->NewEnum(outer, node->name()));
    return;
  }
  if (present->is<sm::Field>()) {
    return Error(ErrorCode::NameTreeEnumDuplicate, node, present->name());
  }
  Error(ErrorCode::NameTreeEnumConflict, node, present->name());
}

void NameTreeBuilder::VisitField(ast::Field* node) {
  auto const owner =
      SemanticOf(node->parent()->as<ast::ClassBody>())->as<sm::Class>();
  auto const present = owner->FindMember(node->name());
  if (!present) {
    editor_->SetSemanticOf(
        node, session()->semantic_factory()->NewField(owner, node->name()));
    return;
  }
  if (present->is<sm::Field>()) {
    return Error(ErrorCode::NameTreeFieldDuplicate, node, present->name());
  }
  Error(ErrorCode::NameTreeFieldConflict, node, present->name());
}

void NameTreeBuilder::VisitMethod(ast::Method* node) {
  auto const owner =
      SemanticOf(node->parent()->as<ast::ClassBody>())->as<sm::Class>();
  auto const present = owner->FindMember(node->name());
  if (!present) {
    session()->semantic_factory()->NewMethodGroup(owner, node->name());
    return;
  }
  if (present->is<sm::MethodGroup>())
    return;
  Error(ErrorCode::NameTreeMethodConflict, node->name());
}

void NameTreeBuilder::VisitNamespaceBody(ast::NamespaceBody* node) {
  ProcessNamespaceBody(node);
  ast::Visitor::VisitNamespaceBody(node);
}

}  // namespace compiler
}  // namespace elang
