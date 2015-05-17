// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/compilation_session.h"

#include "base/containers/adapters.h"
#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/analysis_editor.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token_factory.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {

void PopulateSemantics(CompilationSession* session) {
  AnalysisEditor editor(session->analysis());

  auto const global_namespace =
      session->semantics_factory()->global_namespace();
  editor.SetSemanticOf(session->global_namespace(), global_namespace);
  editor.SetSemanticOf(session->global_namespace_body(), global_namespace);

  auto const system_namespace =
      session->semantics_factory()->system_namespace();
  editor.SetSemanticOf(session->system_namespace(), system_namespace);
  editor.SetSemanticOf(session->system_namespace_body(), system_namespace);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
CompilationSession::CompilationSession()
    : ErrorSink(zone()),
      analysis_(new Analysis()),
      token_factory_(new TokenFactory(zone())),
      ast_factory_(new ast::Factory(this)),
      semantics_factory_(new sm::Factory(token_factory_.get())) {
  PopulateSemantics(this);
}

CompilationSession::~CompilationSession() {
}

AtomicStringFactory* CompilationSession::atomic_string_factory() const {
  return token_factory_->atomic_string_factory();
}

ast::Namespace* CompilationSession::global_namespace() const {
  return ast_factory_->global_namespace();
}

ast::NamespaceBody* CompilationSession::global_namespace_body() const {
  return ast_factory_->global_namespace_body();
}

ast::Namespace* CompilationSession::system_namespace() const {
  return ast_factory_->system_namespace();
}

ast::NamespaceBody* CompilationSession::system_namespace_body() const {
  return ast_factory_->system_namespace_body();
}

Token* CompilationSession::system_token() const {
  return token_factory_->system_token();
}

AtomicString* CompilationSession::NewAtomicString(base::StringPiece16 string) {
  return token_factory()->NewAtomicString(string);
}

CompilationUnit* CompilationSession::NewCompilationUnit(
    SourceCode* source_code) {
  auto unit = std::make_unique<CompilationUnit>(this, source_code);
  compilation_units_.push_back(std::move(unit));
  return compilation_units_.back().get();
}

base::StringPiece16* CompilationSession::NewString(base::StringPiece16 string) {
  return token_factory_->NewString(string);
}

Token* CompilationSession::NewUniqueNameToken(const SourceCodeRange& location,
                                              const base::char16* format) {
  return token_factory_->NewUniqueNameToken(location, format);
}

Token* CompilationSession::NewToken(const SourceCodeRange& location,
                                    const TokenData& data) {
  return token_factory_->NewToken(location, data);
}

Token* CompilationSession::NewToken(const SourceCodeRange& location,
                                    AtomicString* name) {
  return token_factory_->NewToken(location, TokenData(name));
}

Token* CompilationSession::PredefinedNameOf(PredefinedName name) const {
  return token_factory_->PredefinedNameOf(name);
}

sm::Type* CompilationSession::PredefinedTypeOf(PredefinedName name) {
  auto const name_token = PredefinedNameOf(name);
  auto const present =
      semantics_factory()->system_namespace()->FindMember(name_token);
  if (!present) {
    AddError(ErrorCode::PredefinedNamesNameNotFound, name_token);
    return semantics_factory()->NewUndefinedType(name_token);
  }
  auto const type = present->as<sm::Type>();
  if (!type) {
    AddError(ErrorCode::PredefinedNamesNameNotClass, name_token);
    return semantics_factory()->NewUndefinedType(name_token);
  }
  return type;
}

AtomicString* CompilationSession::QualifiedNameOf(sm::Semantic* node) {
  std::vector<sm::Semantic*> components;
  size_t length = 0;
  size_t dot_length = 0;
  for (auto runner = node; runner && runner->name(); runner = runner->outer()) {
    components.push_back(runner);
    length += dot_length;
    length += runner->name()->atomic_string()->string().size();
    dot_length = 1;
  }
  base::string16 name;
  name.reserve(length);
  auto need_dot = false;
  for (auto const component : base::Reversed(components)) {
    if (need_dot)
      name.append(1, '.');
    component->name()->atomic_string()->string().AppendToString(&name);
    need_dot = true;
  }
  return NewAtomicString(name);
}

ast::NamedNode* CompilationSession::QueryAstNode(
    base::StringPiece16 reference) {
  auto enclosing = static_cast<ast::ContainerNode*>(global_namespace());
  auto found = static_cast<ast::NamedNode*>(nullptr);
  for (size_t pos = 0u; pos < reference.length(); ++pos) {
    auto dot_pos = reference.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = reference.length();
    auto const name = NewAtomicString(reference.substr(pos, dot_pos - pos));
    found = enclosing->FindMember(name);
    if (!found)
      return nullptr;
    pos = dot_pos;
    if (pos == reference.length())
      break;
    enclosing = found->as<ast::NamespaceNode>();
    if (!enclosing)
      return nullptr;
  }
  return found;
}

}  // namespace compiler
}  // namespace elang
