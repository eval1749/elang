// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "elang/lir/dump_function_pass.h"

#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// DumpDumpFunctionPass
//
DumpFunctionPass::DumpFunctionPass(base::StringPiece name, Editor* editor)
    : FunctionPass(name, editor) {
}

DumpFunctionPass::~DumpFunctionPass() {
}

void DumpFunctionPass::RunOnFunction() {
  TextFormatter formatter(factory()->literals(), &std::cout);
  formatter.FormatFunction(function());
}

}  // namespace lir
}  // namespace elang
