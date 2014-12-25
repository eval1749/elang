// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_compilation_session_h)
#define INCLUDE_elang_compiler_compilation_session_h

#include <memory>
#include <vector>
#include <unordered_map>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace elang {

namespace hir {
class Factory;
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

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
class CompilationSession final {
  private: std::unordered_map<base::StringPiece16, base::StringPiece16*>
      atomic_strings_;
  private: const std::unique_ptr<ast::NodeFactory> ast_factory_;
  private: std::vector<std::unique_ptr<CompilationUnit>> compilation_units_;
  private: std::vector<ErrorData*> errors_;
  private: const std::unique_ptr<hir::Factory> hir_factory_;
  // TODO(eval1749) We should use |hir::Factory| for string allocation.
  // Because we want to record local variable name into HIR.
  private: std::vector<base::string16*> strings_;
  private: std::vector<base::StringPiece16*> string_pieces_;

  private: std::unique_ptr<SourceCode> source_code_;
  private: ast::Namespace* const global_namespace_;

  public: CompilationSession();
  public: ~CompilationSession();

  public: ast::NodeFactory* ast_factory() const {
    return ast_factory_.get();
  }
  public: hir::Factory* hir_factory() const { return hir_factory_.get(); }

  // TODO(eval1749) We should sort error list by source code offset.
  public: const std::vector<ErrorData*>& errors() const {
    return errors_;
  }

  public: ast::Namespace* global_namespace() const {
    return global_namespace_;
  }

  public: void AddError(ErrorCode error_code, const Token& token);
  public: void AddError(ErrorCode error_code, const Token& token1,
                        const Token& token2);
  // Lexer uses this.
  public: void AddError(const SourceCodeRange& location, ErrorCode error_code);
  private: void AddError(const SourceCodeRange& location, ErrorCode error_code,
                         const std::vector<Token>& tokens);
  public: base::StringPiece16* GetOrNewAtomicString(base::StringPiece16 string);
  public: CompilationUnit* NewCompilationUnit(SourceCode* source_code);
  public: base::StringPiece16* NewString(base::StringPiece16 string);

  DISALLOW_COPY_AND_ASSIGN(CompilationSession);
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_compilation_session_h)
