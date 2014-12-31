// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_SIMPLE_NAME_H_
#define ELANG_HIR_SIMPLE_NAME_H_

#include <ostream>
#include <string>

#include "base/strings/string_piece.h"
#include "elang/base/zone_object.h"

namespace elang {
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class SimpleName final : ZoneObject {
  friend class Factory;

 public:
  base::StringPiece16 string() const { return string_; }

 private:
  explicit SimpleName(base::StringPiece16 string);
  ~SimpleName() = delete;

  base::StringPiece16 const string_;

  DISALLOW_COPY_AND_ASSIGN(SimpleName);
};

std::ostream& operator<<(std::ostream& ostream, const SimpleName& simple_name);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_SIMPLE_NAME_H_
