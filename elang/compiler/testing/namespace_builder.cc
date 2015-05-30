// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/namespace_builder.h"

#include <algorithm>

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/editor.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

NamespaceBuilder::NamespaceBuilder(NameResolver* name_resolver)
    : CompilationSessionUser(name_resolver->session()),
      analysis_editor_(
          new AnalysisEditor(name_resolver->session()->analysis())),
      name_resolver_(name_resolver),
      semantic_editor_(new sm::Editor(name_resolver->session())) {
}

NamespaceBuilder::~NamespaceBuilder() {
}

sm::Class* NamespaceBuilder::system_object() {
  return PredefinedTypeOf(PredefinedName::Object)->as<sm::Class>();
}

ast::ClassBody* NamespaceBuilder::NewClass(TokenType token_type,
                                           base::StringPiece name,
                                           base::StringPiece base_names) {
  DCHECK(token_type == TokenType::Class || token_type == TokenType::Struct);
  auto const modifiers = Modifiers(Modifier::Public);
  auto const ast_class = session()->ast_factory()->NewClass(
      session()->system_namespace(), modifiers, NewKeyword(token_type),
      NewName(name));
  session()->system_namespace()->AddNamedMember(ast_class);

  std::vector<ast::Type*> base_class_names;
  for (size_t pos = 0; pos < base_names.size(); ++pos) {
    auto const space_pos =
        std::min(base_names.find(' ', pos), base_names.size());
    auto const base_name = base_names.substr(pos, space_pos - pos);
    base_class_names.push_back(NewTypeReference(base_name));
    pos = space_pos;
  }

  auto const ast_class_body = session()->ast_factory()->NewClassBody(
      session()->system_namespace_body(), ast_class, base_class_names);
  session()->system_namespace_body()->AddMember(ast_class_body);
  session()->system_namespace_body()->AddNamedMember(ast_class);

  // sm::Class
  std::vector<sm::Class*> base_classes;
  for (auto const base_name : ast_class_body->base_class_names()) {
    auto const base_class = name_resolver()->ResolveReference(
        base_name, ast_class_body->parent()->as<ast::ContainerNode>());
    DCHECK(base_class->is<sm::Class>()) << *base_class;
    base_classes.push_back(base_class->as<sm::Class>());
  }
  auto const outer = session()->analysis()->SemanticOf(ast_class->parent());
  auto const clazz = token_type == TokenType::Class
                         ? name_resolver()->factory()->NewClass(
                               outer, modifiers, ast_class->name(), ast_class)
                         : name_resolver()->factory()->NewStruct(
                               outer, modifiers, ast_class->name());
  semantic_editor_->FixClassBase(clazz, base_classes);
  analysis_editor_->SetSemanticOf(ast_class, clazz);
  analysis_editor_->SetSemanticOf(ast_class_body, clazz);

  return ast_class_body;
}

ast::ClassBody* NamespaceBuilder::NewClass(base::StringPiece name,
                                           base::StringPiece base_names) {
  return NewClass(TokenType::Class, name, base_names);
}

Token* NamespaceBuilder::NewKeyword(TokenType type) {
  static const char* const keywords[] = {
#define V(Name, string, details) string,
      FOR_EACH_TOKEN(V, V)
#undef V
  };
  auto const name = session()->NewAtomicString(
      base::UTF8ToUTF16(keywords[static_cast<int>(type)]));
  return session()->NewToken(SourceCodeRange(), TokenData(type, name));
}

Token* NamespaceBuilder::NewName(base::StringPiece name) {
  return session()->NewToken(
      SourceCodeRange(), session()->NewAtomicString(base::UTF8ToUTF16(name)));
}

ast::Parameter* NamespaceBuilder::NewParameter(ast::Method* method,
                                               int position,
                                               base::StringPiece type,
                                               base::StringPiece name) {
  return session()->ast_factory()->NewParameter(
      method, ParameterKind::Required, position, NewTypeReference(type),
      NewName(name), nullptr);
}

ast::ClassBody* NamespaceBuilder::NewStruct(base::StringPiece name,
                                            base::StringPiece base_names) {
  return NewClass(TokenType::Struct, name, base_names);
}

ast::Type* NamespaceBuilder::NewTypeReference(TokenType keyword) {
  return session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(NewKeyword(keyword)));
}

ast::Type* NamespaceBuilder::NewTypeReference(base::StringPiece reference) {
  ast::Type* last = nullptr;
  for (size_t pos = 0; pos < reference.size(); ++pos) {
    auto dot_pos = reference.find('.', pos);
    if (dot_pos == std::string::npos)
      dot_pos = reference.size();
    auto const name = NewName(reference.substr(pos, dot_pos - pos));
    pos = dot_pos;
    if (last) {
      last = session()->ast_factory()->NewTypeMemberAccess(
          session()->ast_factory()->NewMemberAccess(last, name));
      continue;
    }
    last = session()->ast_factory()->NewTypeNameReference(
        session()->ast_factory()->NewNameReference(name));
  }
  return last;
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
