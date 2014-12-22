// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/string_source_code.h"

#include "elang/compiler/string_stream.h"

namespace elang {
namespace compiler {

StringSourceCode::StringSourceCode(const base::string16& name,
                                   const base::StringPiece16& string)
    : SourceCode(name), stream_(new StringStream(string)) {
}

StringSourceCode::~StringSourceCode() {
}

}  // namespace compiler
}  // namespace elang
