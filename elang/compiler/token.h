// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_H_
#define ELANG_COMPILER_TOKEN_H_

#include <memory>
#include <ostream>

#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"
#include "elang/base/zone_allocated.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/token_data.h"

namespace elang {
namespace compiler {

class TokenFactory;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// Token
//
class Token final : public ZoneAllocated {
 public:
  bool operator==(const Token& other) const;
  bool operator!=(const Token& other) const;

  AtomicString* atomic_string() const { return data_.atomic_string(); }
  bool bool_data() const { return data_.bool_data(); }
  base::char16 char_data() const { return data_.char_data(); }
  TokenData data() const { return data_; }
  float32_t f32_data() const { return data_.f32_data(); }
  float64_t f64_data() const { return data_.f64_data(); }
  bool has_atomic_string() const { return data_.has_atomic_string(); }
  int16_t int16_data() const;
  int32_t int32_data() const;
  int64_t int64_data() const;
  int8_t int8_data() const;
  bool is_contextual_keyword() const { return data_.is_contextual_keyword(); }
  bool is_keyword() const { return data_.is_keyword(); }
  bool is_left_bracket() const { return data_.is_left_bracket(); }
  bool is_literal() const { return data_.is_literal(); }
  bool is_name() const { return data_.is_name(); }
  bool is_operator() const { return data_.is_operator(); }
  bool is_right_bracket() const { return data_.is_right_bracket(); }
  bool is_type_name() const { return data_.is_type_name(); }
  PredefinedName literal_type() const { return data_.literal_type(); }
  const SourceCodeRange& location() const { return location_; }
  PredefinedName mapped_type_name() const { return data_.mapped_type_name(); }
  int precedence() const { return data_.precedence(); }
  TokenType right_bracket() const { return data_.right_bracket(); }
  base::StringPiece16 string_data() const { return data_.string_data(); }
  TokenType type() const { return data_.type(); }
  uint16_t uint16_data() const;
  uint32_t uint32_data() const;
  uint64_t uint64_data() const { return data_.uint64_data(); }
  uint8_t uint8_data() const;

 private:
  friend class TokenFactory;

  Token(const SourceCodeRange& source_range, const TokenData& data);
  ~Token() = delete;

  const TokenData data_;
  const SourceCodeRange location_;

  DISALLOW_COPY_AND_ASSIGN(Token);
};

// Convenience function to allow |PeekType() == TokenType::Var|.
inline bool operator==(const Token* token, TokenType type) {
  return token->type() == type;
}

inline bool operator!=(const Token* token, TokenType type) {
  return !operator==(token, type);
}

std::ostream& operator<<(std::ostream& ostream, const Token& token);
std::ostream& operator<<(std::ostream& ostream, const Token* token);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_H_
