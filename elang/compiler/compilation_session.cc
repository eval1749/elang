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
#include "elang/compiler/ast/visitor.h"
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

  auto const global_namespace = session->semantic_factory()->global_namespace();
  editor.SetSemanticOf(session->global_namespace_body(), global_namespace);

  auto const system_namespace = session->semantic_factory()->system_namespace();
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
      semantic_factory_(new sm::Factory(token_factory_.get())) {
  PopulateSemantics(this);
}

CompilationSession::~CompilationSession() {
}

AtomicStringFactory* CompilationSession::atomic_string_factory() const {
  return token_factory_->atomic_string_factory();
}

ast::NamespaceBody* CompilationSession::global_namespace_body() const {
  return ast_factory_->global_namespace_body();
}

ast::NamespaceBody* CompilationSession::system_namespace_body() const {
  return ast_factory_->system_namespace_body();
}

Token* CompilationSession::system_token() const {
  return token_factory_->system_token();
}

void CompilationSession::Apply(ast::Visitor* visitor) {
  for (auto const& compilation_unit : compilation_units_)
    visitor->Traverse(compilation_unit->namespace_body());
}

AtomicString* CompilationSession::NewAtomicString(base::StringPiece16 string) {
  return token_factory()->NewAtomicString(string);
}

CompilationUnit* CompilationSession::NewCompilationUnit(
    SourceCode* source_code) {
  auto const namespace_body = ast_factory()->NewNamespaceBody(
      nullptr, ast_factory()->global_namespace());
  auto unit = std::make_unique<CompilationUnit>(namespace_body, source_code);
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
      semantic_factory()->system_namespace()->FindMember(name_token);
  if (!present) {
    AddError(ErrorCode::PredefinedNamesNameNotFound, name_token);
    return semantic_factory()->NewUndefinedType(name_token);
  }
  auto const type = present->as<sm::Type>();
  if (!type) {
    AddError(ErrorCode::PredefinedNamesNameNotClass, name_token);
    return semantic_factory()->NewUndefinedType(name_token);
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

ast::Node* CompilationSession::QueryAstNode(base::StringPiece16 path) {
  struct Local {
    static ast::Node* Find(const std::vector<AtomicString*> names,
                           size_t position,
                           ast::Node* node) {
      if (node->name()->atomic_string() != names[position])
        return nullptr;
      auto const next_position = position + 1;
      if (next_position == names.size())
        return node;
      auto const container = node->as<ast::ContainerNode>();
      if (!container)
        return nullptr;
      for (auto const member : container->members()) {
        if (auto const found = Find(names, next_position, member))
          return found;
      }
      return nullptr;
    }
  };
  std::vector<AtomicString*> names;
  for (size_t pos = 0u; pos < path.length(); ++pos) {
    auto dot_pos = path.find('.', pos);
    if (dot_pos == base::StringPiece16::npos)
      dot_pos = path.length();
    names.push_back(NewAtomicString(path.substr(pos, dot_pos - pos)));
    pos = dot_pos;
  }
  if (names.empty())
    return nullptr;
  for (auto const& compilation_unit : compilation_units_) {
    for (auto const member : compilation_unit->namespace_body()->members()) {
      if (auto const found = Local::Find(names, 0, member))
        return found;
    }
  }
  // TODO(eval1749) Once we get rid of |ast::Namespace|, we should remove
  // below code.
  for (auto const member : global_namespace_body()->members()) {
    if (auto const found = Local::Find(names, 0, member))
      return found;
  }
  return nullptr;
}

}  // namespace compiler
}  // namespace elang
