// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_PREDEFINED_TYPES_H_
#define ELANG_COMPILER_ANALYZE_PREDEFINED_TYPES_H_

#include <array>

#include "base/macros.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/predefined_names.h"

namespace elang {
class AtomicString;
namespace compiler {
class CompilationSession;

//////////////////////////////////////////////////////////////////////
//
// PredefinedTypes
//
class PredefinedTypes final {
 public:
  explicit PredefinedTypes(CompilationSession* session);
  ~PredefinedTypes();

  ast::Class* type_from(PredefinedName name) const;

 private:
  std::array<ast::Class*, kNumberOfPredefinedNames> types_;

  DISALLOW_COPY_AND_ASSIGN(PredefinedTypes);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_PREDEFINED_TYPES_H_
