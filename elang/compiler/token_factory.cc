// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iterator>

#include "elang/compiler/token_factory.h"

#include "elang/base/atomic_string_factory.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Token
//
TokenFactory::TokenFactory(Zone* zone)
    : ZoneUser(zone),
      atomic_string_factory_(new AtomicStringFactory()),
      source_code_(new StringSourceCode(L"-", L"")),
      system_token_(NewSystemName(L"System")) {
  const base::char16* names[] = {
#define V(Name) L## #Name,
      FOR_EACH_PREDEFINED_NAME(V)
#undef V
  };
  predefined_names_.reserve(std::end(names) - std::begin(names));
  for (auto it = std::begin(names); it != std::end(names); ++it)
    predefined_names_.push_back(NewSystemName(*it));
}

TokenFactory::~TokenFactory() {
}

SourceCodeRange TokenFactory::internal_code_location() const {
  return SourceCodeRange(source_code_.get(), 0, 0);
}

AtomicString* TokenFactory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

base::StringPiece16* TokenFactory::NewString(base::StringPiece16 string) {
  auto const buffer = atomic_string_factory_->NewString(string);
  return new (zone()->Allocate(sizeof(base::StringPiece16)))
      base::StringPiece16(buffer.data(), buffer.size());
}

Token* TokenFactory::NewSystemKeyword(TokenType type,
                                      base::StringPiece16 name) {
  return NewToken(internal_code_location(),
                  TokenData(type, NewAtomicString(name)));
}

Token* TokenFactory::NewSystemName(base::StringPiece16 name) {
  return NewToken(internal_code_location(), TokenData(NewAtomicString(name)));
}

Token* TokenFactory::NewToken(const SourceCodeRange& source_range,
                              const TokenData& data) {
  return new (zone()) Token(source_range, data);
}

Token* TokenFactory::NewUniqueNameToken(const SourceCodeRange& location,
                                        const base::char16* format) {
  auto const name = atomic_string_factory_->NewUniqueAtomicString(format);
  return NewToken(location, TokenData(TokenType::TempName, name));
}

Token* TokenFactory::PredefinedNameOf(PredefinedName name) const {
  return predefined_names_[static_cast<size_t>(name)];
}

}  // namespace compiler
}  // namespace elang
