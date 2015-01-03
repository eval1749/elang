// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ATOMIC_STRING_H_
#define ELANG_BASE_ATOMIC_STRING_H_

#include <ostream>
#include <string>

#include "base/strings/string_piece.h"
#include "elang/base/base_export.h"
#include "elang/base/zone_allocated.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// AtomicString
//
class ELANG_BASE_EXPORT AtomicString final : public ZoneAllocated {
 public:
  base::StringPiece16 string() const { return string_; }

 private:
  friend class AtomicStringFactory;

  explicit AtomicString(base::StringPiece16 string);
  ~AtomicString() = delete;

  base::StringPiece16 const string_;

  DISALLOW_COPY_AND_ASSIGN(AtomicString);
};

ELANG_BASE_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                           const AtomicString& simple_name);

}  // namespace elang

#endif  // ELANG_BASE_ATOMIC_STRING_H_
