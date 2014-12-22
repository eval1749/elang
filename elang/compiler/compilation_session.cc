// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_session.h"

#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_data.h"

namespace elang {
namespace compiler {

CompilationSession::CompilationSession() {
}

CompilationSession::~CompilationSession() {
  for (auto string_piece : string_pieces_)
    delete string_piece;
  for (auto string : strings_)
    delete string;
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code) {
  AddError(location, error_code, std::vector<Token>());
}

void CompilationSession::AddError(const SourceCodeRange& location,
                                  ErrorCode error_code,
                                  const std::vector<Token>& tokens) {
  auto error_data = std::make_unique<ErrorData>(location, error_code, tokens);
  errors_.push_back(std::move(error_data));
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
  auto unit = std::make_unique<CompilationUnit>(source_code);
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
