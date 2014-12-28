// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_COMPILATION_SESSION_H_
#define ELANG_COMPILER_COMPILATION_SESSION_H_

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/compiler/token_data.h"

namespace elang {

namespace hir {
class Factory;
class SimpleName;
}

namespace compiler {

namespace ast {
class Namespace;
class NodeFactory;
}

class CompilationUnit;
enum class ErrorCode;
class ErrorData;
class SourceCode;
class SourceCodeRange;
class Token;
class TokenFactory;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
class CompilationSession final {
 public:
  CompilationSession();
  ~CompilationSession();

  ast::NodeFactory* ast_factory() const { return ast_factory_.get(); }
  hir::Factory* hir_factory() const { return hir_factory_.get(); }
  // TODO(eval1749) We should sort error list by source code offset.
  const std::vector<ErrorData*>& errors() const { return errors_; }
  ast::Namespace* global_namespace() const { return global_namespace_; }

  void AddError(ErrorCode error_code, Token* token);
  void AddError(ErrorCode error_code, Token* token1, Token* token2);
  // Lexer uses this.
  void AddError(const SourceCodeRange& location, ErrorCode error_code);
  hir::SimpleName* GetOrCreateSimpleName(base::StringPiece16 string);
  CompilationUnit* NewCompilationUnit(SourceCode* source_code);
  base::StringPiece16* NewString(base::StringPiece16 string);
  Token* NewUniqueNameToken(const SourceCodeRange& location,
                            const base::char16* format);
  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);

 private:
  void AddError(const SourceCodeRange& location,
                ErrorCode error_code,
                const std::vector<Token*>& tokens);

  const std::unique_ptr<ast::NodeFactory> ast_factory_;
  std::vector<std::unique_ptr<CompilationUnit>> compilation_units_;
  std::vector<ErrorData*> errors_;
  const std::unique_ptr<hir::Factory> hir_factory_;
  std::vector<base::StringPiece16*> string_pool_;
  std::unique_ptr<TokenFactory> token_factory_;

  std::unique_ptr<SourceCode> source_code_;
  ast::Namespace* const global_namespace_;

  DISALLOW_COPY_AND_ASSIGN(CompilationSession);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_SESSION_H_
