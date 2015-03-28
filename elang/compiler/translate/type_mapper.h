// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TRANSLATE_TYPE_MAPPER_H_
#define ELANG_COMPILER_TRANSLATE_TYPE_MAPPER_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
class AtomicString;

namespace optimizer {
class Type;
class TypeFactory;
}

namespace compiler {
namespace ast {
class NamedNode;
}
namespace sm {
class Type;
}
enum class PredefinedName;
class Semantics;

namespace cm = compiler;
namespace ir = optimizer;

//////////////////////////////////////////////////////////////////////
//
// IrTypeMapper maps semantic type to IR type.
//
class IrTypeMapper final : public cm::CompilationSessionUser {
 public:
  IrTypeMapper(cm::CompilationSession* session, ir::TypeFactory* factory);
  ~IrTypeMapper();

  // Map semantic type to IR type.
  ir::Type* Map(sm::Type* sm_type);
  ir::Type* Map(cm::PredefinedName name);

 private:
  void InstallType(sm::Type* sm_type, ir::Type* ir_type);

  ir::TypeFactory* const type_factory_;

  std::unordered_map<sm::Type*, ir::Type*> sm_type_map_;

  DISALLOW_COPY_AND_ASSIGN(IrTypeMapper);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TRANSLATE_TYPE_MAPPER_H_
