// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/emitters/code_emitter.h"

namespace elang {
namespace lir {

CodeEmitter::CodeEmitter(Factory* factory, api::MachineCodeBuilder* builder)
    : builder_(builder), factory_(factory) {
}

CodeEmitter::~CodeEmitter() {
}

}  // namespace lir
}  // namespace elang
