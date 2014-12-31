// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_ATOMIC_STRING_H_
#define ELANG_HIR_ATOMIC_STRING_H_

#include <ostream>
#include <string>

#include "base/strings/string_piece.h"
#include "elang/base/zone_object.h"

namespace elang {
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
class AtomicString final : public ZoneObject {
 public:
  base::StringPiece16 string() const { return string_; }

 private:
  friend class Factory;

  explicit AtomicString(base::StringPiece16 string);
  ~AtomicString() = delete;

  base::StringPiece16 const string_;

  DISALLOW_COPY_AND_ASSIGN(AtomicString);
};

std::ostream& operator<<(std::ostream& ostream,
                         const AtomicString& simple_name);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_ATOMIC_STRING_H_
