// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/namespace_builder.h"

#include <algorithm>

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

NamespaceBuilder::NamespaceBuilder(NameResolver* name_resolver)
    : CompilationSessionUser(name_resolver->session()),
      name_resolver_(name_resolver) {
}

NamespaceBuilder::~NamespaceBuilder() {
}

sm::Class* NamespaceBuilder::system_object() {
  auto const ast_class = GetPredefinedType(PredefinedName::Object);
  return semantics()->ValueOf(ast_class)->as<sm::Class>();
}

ast::ClassBody* NamespaceBuilder::NewClass(base::StringPiece name,
                                           base::StringPiece base_names) {
  auto const ast_class = session()->ast_factory()->NewClass(
      session()->system_namespace(), Modifiers(Modifier::Public),
      NewKeyword(TokenType::Class), NewName(name));
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
      session()->system_namespace_body(), ast_class);
  ast_class_body->SetBaseClassNames(base_class_names);
  session()->system_namespace_body()->AddMember(ast_class_body);
  session()->system_namespace_body()->AddNamedMember(ast_class);

  // sm::Class
  std::vector<sm::Class*> base_classes;
  for (auto const base_name : ast_class_body->base_class_names()) {
    auto const ast_base_class =
        name_resolver()->ResolveReference(base_name, ast_class_body->parent());
    DCHECK(ast_base_class) << " Not found " << *base_name;
    auto const base_class =
        name_resolver()->Resolve(ast_base_class)->as<sm::Class>();
    DCHECK(base_class);
    base_classes.push_back(base_class);
  }
  auto const clazz =
      name_resolver()->factory()->NewClass(ast_class, base_classes);
  name_resolver()->DidResolve(ast_class, clazz);
  name_resolver()->DidResolve(ast_class_body, clazz);

  return ast_class_body;
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

ast::Type* NamespaceBuilder::NewTypeReference(TokenType keyword) {
  return session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(NewKeyword(keyword)));
}

ast::Type* NamespaceBuilder::NewTypeReference(base::StringPiece name) {
  std::vector<ast::Expression*> names;
  for (size_t pos = 0; pos < name.size(); ++pos) {
    auto dot_pos = name.find('.', pos);
    if (dot_pos == std::string::npos)
      dot_pos = name.size();
    names.push_back(session()->ast_factory()->NewNameReference(
        NewName(name.substr(pos, dot_pos - pos))));
    pos = dot_pos;
  }
  DCHECK(!names.empty());
  if (names.size() == 1) {
    return session()->ast_factory()->NewTypeNameReference(
        names[0]->as<ast::NameReference>());
  }
  return session()->ast_factory()->NewTypeMemberAccess(
      session()->ast_factory()->NewMemberAccess(names[0]->token(), names));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
