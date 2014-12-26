// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_simple_name_h)
#define INCLUDE_elang_hir_simple_name_h

#include <ostream>

#include "base/strings/string_piece.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class SimpleName final {
  friend class Factory;

  private: base::StringPiece16 const string_;

  private: SimpleName(base::StringPiece16 string);
  private: ~SimpleName();

  public: base::StringPiece16 string() const { return string_; }

  DISALLOW_COPY_AND_ASSIGN(SimpleName);
};

std::ostream& operator<<(std::ostream& ostream, const SimpleName& simple_name);

}  // namespace hir
}  // namespace elang

#endif // !defined(INCLUDE_elang_hir_simple_name_h)
