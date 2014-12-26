// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "elang/compiler/compilation_session.h"

#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/factory.h"

namespace elang {
namespace compiler {

namespace {

ast::Namespace* CreateGlobalNamespace(CompilationSession* session,
                                      SourceCode* source_code) {
  return session->ast_factory()->NewNamespace(nullptr,
      Token(SourceCodeRange(source_code, 0, 0), TokenType::Namespace,
            session->GetOrNewAtomicString(L"namespace")),
      Token(SourceCodeRange(source_code, 0, 0), TokenType::SimpleName,
            session->GetOrNewAtomicString(L"::")));
}

}  // namespace

CompilationSession::CompilationSession()
    : ast_factory_(new ast::NodeFactory()),
      hir_factory_(new hir::Factory()),
      source_code_(new StringSourceCode(L"-", L"")),
      global_namespace_(CreateGlobalNamespace(this, source_code_.get())) {
}

CompilationSession::~CompilationSession() {
  // For ease of debugging AST, we delete AST factory before string pool.
  ast_factory_->RemoveAll();
  for (auto const error : errors_)
    delete error;
  for (auto const string_piece : string_pieces_)
    delete string_piece;
  for (auto const string : strings_)
    delete string;
}

void CompilationSession::AddError(ErrorCode error_code, const Token& token) {
  AddError(token.location(), error_code, std::vector<Token> { token });
}

void CompilationSession::AddError(ErrorCode error_code, const Token& token1,
                                  const Token& token2) {
  AddError(token1.location(), error_code,
           std::vector<Token> { token1, token2 });
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code) {
  AddError(location, error_code, std::vector<Token>());
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code,
                                  const std::vector<Token>& tokens) {
  errors_.push_back(new ErrorData(location, error_code, tokens));
  std::sort(errors_.begin(), errors_.end(),
            [](const ErrorData* a, const ErrorData* b) {
              return a->location().start_offset() < b->location().start_offset();
            });
}

base::StringPiece16* CompilationSession::GetOrNewAtomicString(
    base::StringPiece16 string) {
  auto present = atomic_strings_.find(string);
  if (present != atomic_strings_.end())
    return present->second;
  auto const result = NewString(string);
  atomic_strings_[*result] = result;
  return result;
}

CompilationUnit* CompilationSession::NewCompilationUnit(
    SourceCode* source_code) {
  auto unit = std::make_unique<CompilationUnit>(this, source_code);
  compilation_units_.push_back(std::move(unit));
  return compilation_units_.back().get();
}

base::StringPiece16* CompilationSession::NewString(
    base::StringPiece16 string_piece) {
  auto const string = new base::string16(string_piece.data(),
                                         string_piece.size());
  strings_.push_back(string);
  auto const result = new base::StringPiece16(string->data(), string->size());
  string_pieces_.push_back(result);
  return result;
}

}  // namespace compiler
}  // namespace elang
