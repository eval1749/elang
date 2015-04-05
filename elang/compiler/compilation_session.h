// Copyright 2014-2015 Project Vogue. All rights reserved.
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
#include "elang/base/zone_owner.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/token_data.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;

namespace hir {
class Factory;
class Function;
}

namespace optimizer {
class Factory;
class Function;
}

namespace compiler {
namespace ast {
class Class;
class Factory;
class Method;
class NamedNode;
class Namespace;
class NamespaceBody;
class NodeQuery;
}

namespace sm {
class Factory;
}

class CompilationUnit;
enum class ErrorCode;
class ErrorData;
class NameResolver;
enum class PredefinedName;
class PredefinedNames;
class Semantics;
class SourceCode;
class SourceCodeRange;
class Token;
class TokenFactory;
enum class TokenType;

namespace ir = optimizer;

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
class CompilationSession final : public ZoneOwner {
 public:
  CompilationSession();
  ~CompilationSession();

  ast::Factory* ast_factory() const { return ast_factory_.get(); }
  AtomicStringFactory* atomic_string_factory() const {
    return atomic_string_factory_.get();
  }
  const std::vector<ErrorData*>& errors() const { return errors_; }
  ast::Namespace* global_namespace() const { return global_namespace_; }
  AtomicString* name_for(PredefinedName name) const;
  ast::NamespaceBody* global_namespace_body() const {
    return global_namespace_body_;
  }
  Semantics* semantics() const { return semantics_.get(); }
  sm::Factory* semantics_factory() const { return semantics_factory_.get(); }
  ast::Namespace* system_namespace() const { return system_namespace_; }
  ast::NamespaceBody* system_namespace_body() const {
    return system_namespace_body_;
  }
  const std::vector<ErrorData*>& warnings() const { return warnings_; }

  void AddError(ErrorCode error_code, Token* token);
  void AddError(ErrorCode error_code, Token* token1, Token* token2);
  // Lexer uses this.
  void AddError(const SourceCodeRange& location, ErrorCode error_code);

  // Generate HIR functions. See "compile.cc" for implementation of |Compile()|.
  bool Compile(NameResolver* name_resolver, hir::Factory* factory);
  bool Compile(NameResolver* name_resolver, ir::Factory* factory);

  // Returns |hir::Function| of |method|.
  hir::Function* FunctionOf(ast::Method* method);
  ir::Function* IrFunctionOf(ast::Method* method);

  ast::Class* GetPredefinedType(PredefinedName name);
  AtomicString* NewAtomicString(base::StringPiece16 string);
  CompilationUnit* NewCompilationUnit(SourceCode* source_code);
  // Allocate |base::StringPiece16| object in zone used for string backing
  // store for |TokenData|.
  base::StringPiece16* NewString(base::StringPiece16 string);
  Token* NewUniqueNameToken(const SourceCodeRange& location,
                            const base::char16* format);
  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);
  Token* NewToken(const SourceCodeRange& source_range, AtomicString* name);

  void RegisterFunction(ast::Method* method, hir::Function* function);
  void RegisterFunction(ast::Method* method, ir::Function* function);

  // Returns |ast::Node| which qualified name is |qualified_name|.
  ast::NamedNode* QueryAstNode(base::StringPiece16 qualified_name);

  // Returns list of |ast::Node| matched to |query|.
  std::vector<ast::Node*> QueryAstNodes(const ast::NodeQuery& query);

 private:
  void AddError(const SourceCodeRange& location,
                ErrorCode error_code,
                const std::vector<Token*>& tokens);

  const std::unique_ptr<ast::Factory> ast_factory_;
  const std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  std::vector<std::unique_ptr<CompilationUnit>> compilation_units_;
  std::vector<ErrorData*> errors_;
  // The result of compilation.
  std::unordered_map<ast::Method*, hir::Function*> function_map_;
  std::unordered_map<ast::Method*, ir::Function*> ir_function_map_;
  const std::unique_ptr<PredefinedNames> predefined_names_;
  const std::unique_ptr<Semantics> semantics_;
  const std::unique_ptr<sm::Factory> semantics_factory_;
  const std::unique_ptr<TokenFactory> token_factory_;
  std::vector<ErrorData*> warnings_;

  const std::unique_ptr<SourceCode> source_code_;
  ast::Namespace* const global_namespace_;
  ast::NamespaceBody* const global_namespace_body_;
  ast::Namespace* const system_namespace_;
  ast::NamespaceBody* const system_namespace_body_;

  DISALLOW_COPY_AND_ASSIGN(CompilationSession);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_SESSION_H_
