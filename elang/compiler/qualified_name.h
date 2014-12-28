// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_QUALIFIED_NAME_H_
#define ELANG_COMPILER_QUALIFIED_NAME_H_

#include <vector>

#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// QualifiedName
//
class QualifiedName final {
 public:
  explicit QualifiedName(const std::vector<Token*>& simple_names);
  explicit QualifiedName(Token* simple_name);
  QualifiedName(const QualifiedName& other);
  QualifiedName(QualifiedName&& other);
  ~QualifiedName();

  QualifiedName& operator=(const QualifiedName& other);
  QualifiedName& operator=(QualifiedName&& other);

  Token* simple_name() const { return simple_names_.back(); }
  const std::vector<Token*>& simple_names() const { return simple_names_; }

 private:
  std::vector<Token*> simple_names_;
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_QUALIFIED_NAME_H_
