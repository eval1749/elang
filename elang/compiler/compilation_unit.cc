// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/compilation_unit.h"

#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

CompilationUnit::CompilationUnit(CompilationSession*, SourceCode* source_code)
    : source_code_(source_code) {
}

CompilationUnit::~CompilationUnit() {
}

}  // namespace compiler
}  // namespace elang
