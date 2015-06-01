// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/namespace_builder.h"

#include <algorithm>

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/name_resolver.h"
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

NamespaceBuilder::NamespaceBuilder(NameResolver* resolver)
    : CompilationSessionUser(resolver->session()),
      semantic_editor_(new sm::Editor(resolver->session())) {
}

NamespaceBuilder::~NamespaceBuilder() {
}

sm::Class* NamespaceBuilder::system_object() {
  return PredefinedTypeOf(PredefinedName::Object)->as<sm::Class>();
}

sm::Class* NamespaceBuilder::NewClass(TokenType token_type,
                                      base::StringPiece name,
                                      base::StringPiece base_names) {
  DCHECK(token_type == TokenType::Class || token_type == TokenType::Struct);

  auto const factory = session()->semantic_factory();
  auto const outer = factory->system_namespace();
  auto const class_name = NewName(name);

  std::vector<sm::Class*> base_classes;
  for (size_t pos = 0; pos < base_names.size(); ++pos) {
    auto const space_pos =
        std::min(base_names.find(' ', pos), base_names.size());
    auto const base_name = NewName(base_names.substr(pos, space_pos - pos));
    auto const base_class = outer->FindMember(base_name)->as<sm::Class>();
    DCHECK(base_class) << base_name;
    base_classes.push_back(base_class);
    pos = space_pos;
  }

  auto const modifiers = Modifiers(Modifier::Public);
  auto const clazz = token_type == TokenType::Class
                         ? factory->NewClass(outer, modifiers, class_name)
                         : factory->NewStruct(outer, modifiers, class_name);
  semantic_editor_->FixClassBase(clazz, base_classes);

  return clazz;
}

sm::Class* NamespaceBuilder::NewClass(base::StringPiece name,
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

sm::Parameter* NamespaceBuilder::NewParameter(ParameterKind kind,
                                              int position,
                                              base::StringPiece type,
                                              base::StringPiece name) {
  return session()->semantic_factory()->NewParameter(
      kind, position, SemanticOf(type)->as<sm::Type>(), NewName(name), nullptr);
}

sm::Class* NamespaceBuilder::NewStruct(base::StringPiece name,
                                       base::StringPiece base_names) {
  return NewClass(TokenType::Struct, name, base_names);
}

sm::Semantic* NamespaceBuilder::SemanticOf(base::StringPiece16 path) const {
  sm::Semantic* enclosing = session()->semantic_factory()->global_namespace();
  sm::Semantic* found = static_cast<sm::Semantic*>(nullptr);
  for (size_t pos = 0u; pos < path.length(); ++pos) {
    auto dot_pos = path.find('.', pos);
    if (dot_pos == base::StringPiece16::npos)
      dot_pos = path.length();
    auto const name =
        session()->NewAtomicString(path.substr(pos, dot_pos - pos));
    found = enclosing->FindMember(name);
    if (!found)
      return nullptr;
    pos = dot_pos;
    if (pos == path.length())
      break;
    enclosing = found;
    if (!enclosing)
      return nullptr;
  }
  return found;
}

sm::Semantic* NamespaceBuilder::SemanticOf(base::StringPiece path) const {
  return SemanticOf(base::UTF8ToUTF16(path));
}

}  // namespace compiler
}  // namespace elang
