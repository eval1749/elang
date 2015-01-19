// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_DATA_H_
#define ELANG_COMPILER_TOKEN_DATA_H_

#include <ostream>

#include "base/strings/string_piece.h"
#include "elang/base/float_types.h"

namespace elang {
class AtomicString;
namespace compiler {

enum class PredefinedName;
class Token;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// TokenData
//
class TokenData {
 public:
  TokenData(TokenType type, AtomicString* name);
  TokenData(TokenType type, uint64_t u64);
  explicit TokenData(TokenType type);
  explicit TokenData(float32_t f32);
  explicit TokenData(float64_t f64);
  explicit TokenData(AtomicString* name);
  explicit TokenData(base::StringPiece16* str);
  TokenData(const TokenData& other);
  ~TokenData();

  TokenData& operator=(const TokenData& other);
  bool operator==(const TokenData& other) const;
  bool operator!=(const TokenData& other) const;

  AtomicString* atomic_string() const;
  bool bool_data() const;
  base::char16 char_data() const;
  float32_t f32_data() const;
  float64_t f64_data() const;
  bool has_atomic_string() const;
  int64_t int64_data() const;
  bool is_contextual_keyword() const;
  bool is_keyword() const;
  bool is_left_bracket() const;
  bool is_literal() const;
  bool is_name() const;
  bool is_operator() const;
  bool is_right_bracket() const;
  bool is_type_name() const;
  PredefinedName literal_type() const;
  PredefinedName mapped_type_name() const;
  int precedence() const;
  TokenType right_bracket() const;
  base::StringPiece16 string_data() const;
  TokenType type() const { return type_; }

 private:
  union Data {
    base::char16 ch;
    float32_t f32;
    float64_t f64;
    int64_t i64;
    AtomicString* name;
    base::StringPiece16* str;
    uint64_t u64;
  };

  static_assert(sizeof(Data) == sizeof(uint64_t),
                "sizeof(TokenData) must equal to sizeof(uint64_t).");

  bool has_int_data() const;
  bool has_string_data() const;

  Data data_;
  TokenType type_;
};

std::ostream& operator<<(std::ostream& ostream, const TokenData& token);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_DATA_H_
