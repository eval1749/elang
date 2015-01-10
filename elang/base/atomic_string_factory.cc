// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/base/atomic_string_factory.h"

#include "base/strings/stringprintf.h"
#include "elang/base/atomic_string.h"
#include "elang/base/zone.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// AtomicStringFactory
//
AtomicStringFactory::AtomicStringFactory() : unique_name_counter_(0) {
}

AtomicStringFactory::~AtomicStringFactory() {
}

AtomicString* AtomicStringFactory::NewAtomicString(base::StringPiece16 string) {
  auto const it = map_.find(string);
  if (it != map_.end())
    return it->second;
  auto const atomic_string = new (zone()) AtomicString(NewString(string));
  map_[atomic_string->string()] = atomic_string;
  return atomic_string;
}

base::StringPiece16 AtomicStringFactory::NewString(
    base::StringPiece16 string_piece) {
  auto const size = string_piece.size() * sizeof(base::char16);
  auto const string = static_cast<base::char16*>(Allocate(size));
  ::memcpy(string, string_piece.data(), size);
  return base::StringPiece16(string, string_piece.size());
}

AtomicString* AtomicStringFactory::NewUniqueAtomicString(
    const base::char16* format) {
  for (;;) {
    auto const string = base::StringPrintf(format, ++unique_name_counter_);
    auto const it = map_.find(string);
    if (it == map_.end())
      return NewAtomicString(string);
  }
}

}  // namespace elang
