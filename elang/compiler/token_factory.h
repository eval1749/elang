// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_FACTORY_H_
#define ELANG_COMPILER_TOKEN_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_user.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;
class Zone;

namespace compiler {
enum class PredefinedName;
class PredefinedNames;
class SourceCode;
class SourceCodeRange;
class Token;
class TokenData;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// TokenFactory
//
class TokenFactory final : public ZoneUser {
 public:
  explicit TokenFactory(Zone* zone);
  ~TokenFactory();

  AtomicStringFactory* atomic_string_factory() const {
    return atomic_string_factory_.get();
  }
  Token* system_token() const { return system_token_; }

  AtomicString* NewAtomicString(base::StringPiece16 string);

  // Allocate |base::StringPiece16| object in zone used for string backing
  // store for |TokenData|.
  base::StringPiece16* NewString(base::StringPiece16 string);

  // Returns a token for referencing system object, e.g. System.Object.
  Token* NewSystemKeyword(TokenType type, base::StringPiece16 name);
  Token* NewSystemName(base::StringPiece16 name);

  Token* NewToken(const SourceCodeRange& source_range, const TokenData& data);

  // Returns new unique name token.
  Token* NewUniqueNameToken(const SourceCodeRange& location,
                            const base::char16* format);

  Token* PredefinedNameOf(PredefinedName name) const;

 private:
  SourceCodeRange internal_code_location() const;

  const std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  std::vector<Token*> predefined_names_;
  const std::unique_ptr<SourceCode> source_code_;
  Token* const system_token_;

  DISALLOW_COPY_AND_ASSIGN(TokenFactory);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_FACTORY_H_
