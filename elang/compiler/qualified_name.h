// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(elang_compiler_qualified_name_h)
#define elang_compiler_qualified_name_h

#include <vector>

#include "elang/compiler/token.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// QualifiedName
//
class QualifiedName final {
  private: std::vector<Token> simple_names_;

  public: explicit QualifiedName(const std::vector<Token>& simple_names);
  public: explicit QualifiedName(const Token& simple_name);
  public: QualifiedName(const QualifiedName& other);
  public: QualifiedName(QualifiedName&& other);
  public: ~QualifiedName();

  public: QualifiedName& operator=(const QualifiedName& other);
  public: QualifiedName& operator=(QualifiedName&& other);

  public: const Token& simple_name() const { return simple_names_.back(); }
  public: const std::vector<Token> simple_names() const {
    return simple_names_;
  }
};

}  // namespace compiler
}  // namespace elang

#endif // !defined(elang_compiler_qualified_name_h)

