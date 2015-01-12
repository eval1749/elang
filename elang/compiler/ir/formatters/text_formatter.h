// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_FORMATTERS_TEXT_FORMATTER_H_
#define ELANG_COMPILER_IR_FORMATTERS_TEXT_FORMATTER_H_

#include <ostream>

#include "base/macros.h"

namespace elang {
namespace compiler {
namespace ir {

class Node;

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
class TextFormatter final {
 public:
  explicit TextFormatter(std::ostream* stream);
  ~TextFormatter();

  void Format(const ir::Node* node);

 private:
  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(TextFormatter);
};

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_FORMATTERS_TEXT_FORMATTER_H_
