// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_CG_TYPE_MAPPER_H_
#define ELANG_COMPILER_CG_TYPE_MAPPER_H_

#include <unordered_map>

#include "base/macros.h"

namespace elang {
class AtomicString;

namespace hir {
class Factory;
class Type;
}

namespace compiler {

namespace ast {
class NamedNode;
}

namespace ir {
class Type;
}

class CompilationSession;
enum class PredefinedName;
class Semantics;

//////////////////////////////////////////////////////////////////////
//
// TypeMapper
//
class TypeMapper final {
 public:
  TypeMapper(CompilationSession* session, hir::Factory* factory);
  ~TypeMapper();

  // Map IR type to HIR type.
  hir::Type* Map(ir::Type* type);
  hir::Type* Map(PredefinedName name);

 private:
  hir::Factory* factory() const { return factory_; }
  Semantics* semantics() const;
  CompilationSession* session() const { return session_; }

  void InstallType(ir::Type* type, hir::Type* hir_type);

  hir::Factory* const factory_;
  CompilationSession* const session_;

  std::unordered_map<ir::Type*, hir::Type*> type_map_;

  DISALLOW_COPY_AND_ASSIGN(TypeMapper);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_TYPE_MAPPER_H_
