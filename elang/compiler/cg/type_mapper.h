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
class NameResolver;

//////////////////////////////////////////////////////////////////////
//
// TypeMapper
//
class TypeMapper final {
 public:
  TypeMapper(hir::Factory* factory, NameResolver* name_resolver);
  ~TypeMapper();

  hir::Type* Map(ir::Type* type);

 private:
  hir::Factory* factory() { return factory_; }
  NameResolver* name_resolver() { return name_resolver_; }
  CompilationSession* session();

  void InstallType(ir::Type* type, hir::Type* hir_type);

  hir::Factory* const factory_;
  NameResolver* const name_resolver_;

  std::unordered_map<ir::Type*, hir::Type*> type_map_;

  DISALLOW_COPY_AND_ASSIGN(TypeMapper);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_CG_TYPE_MAPPER_H_
