// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

std::ostream& operator<<(std::ostream& ostream, TokenType token_type) {
  static const char* const print_names[] = {
    #define V(name, string, details) string,
    TOKEN_LIST(V, V)
    #undef V
  };
  return ostream << "TokenType::" << print_names[static_cast<int>(token_type)];
}

}  // namespace compiler
}  // namespace elang
