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
#include "elang/compiler/error_sink.h"
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

class Analysis;
class CompilationUnit;
class NameResolver;
enum class PredefinedName;
class Token;
class TokenFactory;
enum class TokenType;

namespace ir = optimizer;

//////////////////////////////////////////////////////////////////////
//
// CompilationSession
//
class CompilationSession final : public ZoneOwner, public ErrorSink {
 public:
  CompilationSession();
  ~CompilationSession();

  // Token
  AtomicStringFactory* atomic_string_factory() const;
  Token* system_token() const;
  TokenFactory* token_factory() const { return token_factory_.get(); }

  // AST nodes
  ast::Factory* ast_factory() const { return ast_factory_.get(); }
  ast::Namespace* global_namespace() const;
  ast::NamespaceBody* global_namespace_body() const;
  ast::Namespace* system_namespace() const;
  ast::NamespaceBody* system_namespace_body() const;

  // Semantic
  Analysis* analysis() const { return analysis_.get(); }
  sm::Factory* semantic_factory() const { return semantic_factory_.get(); }

  // Generate HIR functions. See "compile.cc" for implementation of |Compile()|.
  void Compile(NameResolver* name_resolver, hir::Factory* factory);
  void Compile(NameResolver* name_resolver, ir::Factory* factory);

  // Returns |hir::Function| of |method|.
  hir::Function* FunctionOf(ast::Method* method);
  ir::Function* IrFunctionOf(ast::Method* method);

  AtomicString* NewAtomicString(base::StringPiece16 string);
  CompilationUnit* NewCompilationUnit(SourceCode* source_code);
  // Allocate |base::StringPiece16| object in zone used for string backing
  // store for |TokenData|.
  base::StringPiece16* NewString(base::StringPiece16 string);
  Token* NewUniqueNameToken(const SourceCodeRange& location,
                            const base::char16* format);
  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);
  Token* NewToken(const SourceCodeRange& source_range, AtomicString* name);

  Token* PredefinedNameOf(PredefinedName name) const;

  // Returns predefined type as |ast::Class| of |name|.
  sm::Type* PredefinedTypeOf(PredefinedName name);

  void RegisterFunction(ast::Method* method, hir::Function* function);
  void RegisterFunction(ast::Method* method, ir::Function* function);

  AtomicString* QualifiedNameOf(sm::Semantic* node);

  // Returns |ast::Node| which qualified name is |qualified_name|.
  ast::NamedNode* QueryAstNode(base::StringPiece16 qualified_name);

  // Returns list of |ast::Node| matched to |query|.
  std::vector<ast::Node*> QueryAstNodes(const ast::NodeQuery& query);

 private:
  std::unique_ptr<Analysis> analysis_;
  std::vector<std::unique_ptr<CompilationUnit>> compilation_units_;
  // The result of compilation.
  std::unordered_map<ast::Method*, hir::Function*> function_map_;
  std::unordered_map<ast::Method*, ir::Function*> ir_function_map_;

  const std::unique_ptr<TokenFactory> token_factory_;

  // |ast::Factory| Factory| depends on |TokenFactory|.
  const std::unique_ptr<ast::Factory> ast_factory_;

  // |sm::Factory| Factory| depends on |TokenFactory|.
  const std::unique_ptr<sm::Factory> semantic_factory_;

  DISALLOW_COPY_AND_ASSIGN(CompilationSession);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_COMPILATION_SESSION_H_
