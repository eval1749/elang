// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>

#include "elang/compiler/compilation_session.h"

#include "elang/base/zone.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token_factory.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/factory.h"

namespace elang {
namespace compiler {

namespace {

ast::Namespace* CreateGlobalNamespace(CompilationSession* session,
                                      SourceCode* source_code) {
  return session->ast_factory()->NewNamespace(
      nullptr, session->NewToken(
                   SourceCodeRange(source_code, 0, 0),
                   TokenData(TokenType::Namespace,
                             session->GetOrCreateAtomicString(L"namespace"))),
      session->NewToken(SourceCodeRange(source_code, 0, 0),
                        TokenData(session->GetOrCreateAtomicString(L"::"))));
}

}  // namespace

CompilationSession::CompilationSession()
    : zone_(new Zone()),
      ast_factory_(new ast::NodeFactory(zone_.get())),
      hir_factory_(new hir::Factory()),
      source_code_(new StringSourceCode(L"-", L"")),
      token_factory_(new TokenFactory(zone_.get())),
      global_namespace_(CreateGlobalNamespace(this, source_code_.get())) {
}

CompilationSession::~CompilationSession() {
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
  errors_.push_back(new (zone_.get())
                        ErrorData(zone_.get(), location, error_code, tokens));
  std::sort(errors_.begin(), errors_.end(),
            [](const ErrorData* a, const ErrorData* b) {
    return a->location().start_offset() < b->location().start_offset();
  });
}

hir::AtomicString* CompilationSession::GetOrCreateAtomicString(
    base::StringPiece16 string) {
  return hir_factory()->GetOrCreateAtomicString(string);
}

CompilationUnit* CompilationSession::NewCompilationUnit(
    SourceCode* source_code) {
  auto unit = std::make_unique<CompilationUnit>(this, source_code);
  compilation_units_.push_back(std::move(unit));
  return compilation_units_.back().get();
}

base::StringPiece16* CompilationSession::NewString(base::StringPiece16 string) {
  auto const buffer = hir_factory_->NewString(string);
  return new (zone_->Allocate(sizeof(base::StringPiece16)))
      base::StringPiece16(buffer.data(), buffer.size());
}

Token* CompilationSession::NewUniqueNameToken(const SourceCodeRange& location,
                                              const base::char16* format) {
  return token_factory_->NewToken(
      location,
      TokenData(TokenType::TempName, hir_factory()->NewUniqueName(format)));
}

Token* CompilationSession::NewToken(const SourceCodeRange& location,
                                    const TokenData& data) {
  return token_factory_->NewToken(location, data);
}

}  // namespace compiler
}  // namespace elang
