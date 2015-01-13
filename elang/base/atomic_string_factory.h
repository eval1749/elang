// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_ATOMIC_STRING_FACTORY_H_
#define ELANG_BASE_ATOMIC_STRING_FACTORY_H_

#include <memory>
#include <string>

#include "base/strings/string_piece.h"
#include "elang/base/base_export.h"
#include "elang/base/zone_owner.h"

namespace elang {

class AtomicString;

//////////////////////////////////////////////////////////////////////
//
// AtomicStringFactory
//
class ELANG_BASE_EXPORT AtomicStringFactory final : public ZoneOwner {
 public:
  AtomicStringFactory();
  ~AtomicStringFactory();

  AtomicString* NewAtomicString(base::StringPiece16 string);
  base::StringPiece16 NewString(base::StringPiece16 string);
  AtomicString* NewUniqueAtomicString(const base::char16* format);

 private:
  std::unordered_map<base::StringPiece16, AtomicString*> map_;
  int unique_name_counter_;

  DISALLOW_COPY_AND_ASSIGN(AtomicStringFactory);
};

}  // namespace elang

#endif  // ELANG_BASE_ATOMIC_STRING_FACTORY_H_
