// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_qualified_name_h)
#define INCLUDE_elang_compiler_qualified_name_h

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

#endif  // !defined(INCLUDE_elang_compiler_qualified_name_h)

