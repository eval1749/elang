// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/testing/namespace_builder.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

NamespaceBuilder::NamespaceBuilder(NameResolver* name_resolver)
    : name_resolver_(name_resolver) {
}

NamespaceBuilder::~NamespaceBuilder() {
}

CompilationSession* NamespaceBuilder::session() {
  return name_resolver_->session();
}

ast::Namespace* NamespaceBuilder::system_namespace() {
  return session()->system_namespace();
}

ir::Class* NamespaceBuilder::system_object() {
  auto const ast_class = GetPredefinedType(PredefinedName::Object);
  return name_resolver()->Resolve(ast_class)->as<ir::Class>();
}

ast::Class* NamespaceBuilder::GetPredefinedType(PredefinedName name) {
  return session()
      ->system_namespace()
      ->FindMember(session()->name_for(name))
      ->as<ast::Class>();
}

Token* NamespaceBuilder::NewKeyword(TokenType type) {
  static const char* const keywords[] = {
#define V(Name, string, details) string,
      FOR_EACH_TOKEN(V, IGNORE_TOKEN)
#undef V
  };
  auto const name = session()->NewAtomicString(
      base::UTF8ToUTF16(keywords[static_cast<int>(type) -
                                 static_cast<int>(TokenType::Abstract)]));
  return session()->NewToken(SourceCodeRange(), TokenData(type, name));
}

Token* NamespaceBuilder::NewName(base::StringPiece name) {
  return session()->NewToken(
      SourceCodeRange(), session()->NewAtomicString(base::UTF8ToUTF16(name)));
}

ast::Variable* NamespaceBuilder::NewParameter(base::StringPiece type,
                                              base::StringPiece name) {
  return session()->ast_factory()->NewVariable(nullptr, NewTypeReference(type),
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
        names.front()->as<ast::NameReference>());
  }
  return session()->ast_factory()->NewTypeMemberAccess(
      session()->ast_factory()->NewMemberAccess(nullptr, names));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
