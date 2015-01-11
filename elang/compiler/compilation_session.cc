// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/compiler/compilation_session.h"

#include "elang/base/atomic_string_factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token_factory.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

namespace {

ast::Namespace* CreateGlobalNamespace(CompilationSession* session,
                                      SourceCode* source_code) {
  auto const keyword = session->NewToken(
      SourceCodeRange(source_code, 0, 0),
      TokenData(TokenType::Namespace, session->NewAtomicString(L"namespace")));

  return session->ast_factory()->NewNamespace(
      nullptr, keyword, session->NewToken(keyword->location(),
                                          session->NewAtomicString(L"global")));
}

ast::Namespace* CreateNamespace(CompilationSession* session,
                                ast::NamespaceBody* ns_body,
                                base::StringPiece16 name) {
  auto const enclosing_ns = ns_body->owner();
  auto const ns = session->ast_factory()->NewNamespace(
      enclosing_ns, enclosing_ns->keyword(),
      session->NewToken(enclosing_ns->keyword()->location(),
                        session->NewAtomicString(name)));
  enclosing_ns->AddNamedMember(ns);
  return ns;
}

ast::NamespaceBody* CreateRootNode(CompilationSession* session,
                                   ast::Namespace* global_ns) {
  auto const ns_body =
      session->ast_factory()->NewNamespaceBody(nullptr, global_ns);
  global_ns->AddMember(ns_body);
  return ns_body;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
CompilationSession::CompilationSession()
    : ast_factory_(new ast::NodeFactory(zone())),
      atomic_string_factory_(new AtomicStringFactory()),
      predefined_names_(new PredefinedNames(this)),
      source_code_(new StringSourceCode(L"-", L"")),
      token_factory_(new TokenFactory(zone())),
      global_namespace_(CreateGlobalNamespace(this, source_code_.get())),
      root_node_(CreateRootNode(this, global_namespace())),
      system_namespace_(CreateNamespace(this, root_node(), L"System")) {
}

CompilationSession::~CompilationSession() {
}

AtomicString* CompilationSession::name_for(PredefinedName name) const {
  return predefined_names_->name_for(name);
}

void CompilationSession::AddError(ErrorCode error_code, Token* token) {
  AddError(token->location(), error_code, std::vector<Token*>{token});
}

void CompilationSession::AddError(ErrorCode error_code,
                                  Token* token1,
                                  Token* token2) {
  AddError(token1->location(), error_code, std::vector<Token*>{token1, token2});
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code) {
  AddError(location, error_code, std::vector<Token*>());
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code,
                                  const std::vector<Token*>& tokens) {
  std::vector<ErrorData*>* list =
      error_code > ErrorCode::WarningCodeZero ? &warnings_ : &errors_;
  list->push_back(new (zone()) ErrorData(zone(), location, error_code, tokens));
  std::sort(list->begin(), list->end(),
            [](const ErrorData* a, const ErrorData* b) {
    return a->location().start_offset() < b->location().start_offset();
  });
}

AtomicString* CompilationSession::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

CompilationUnit* CompilationSession::NewCompilationUnit(
    SourceCode* source_code) {
  auto unit = std::make_unique<CompilationUnit>(this, source_code);
  compilation_units_.push_back(std::move(unit));
  return compilation_units_.back().get();
}

base::StringPiece16* CompilationSession::NewString(base::StringPiece16 string) {
  auto const buffer = atomic_string_factory_->NewString(string);
  return new (Allocate(sizeof(base::StringPiece16)))
      base::StringPiece16(buffer.data(), buffer.size());
}

Token* CompilationSession::NewUniqueNameToken(const SourceCodeRange& location,
                                              const base::char16* format) {
  auto const name = atomic_string_factory_->NewUniqueAtomicString(format);
  return token_factory_->NewToken(location,
                                  TokenData(TokenType::TempName, name));
}

Token* CompilationSession::NewToken(const SourceCodeRange& location,
                                    const TokenData& data) {
  return token_factory_->NewToken(location, data);
}

Token* CompilationSession::NewToken(const SourceCodeRange& location,
                                    AtomicString* name) {
  return token_factory_->NewToken(location, TokenData(name));
}

}  // namespace compiler
}  // namespace elang
