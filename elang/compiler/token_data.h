// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TOKEN_DATA_H_
#define ELANG_COMPILER_TOKEN_DATA_H_

#include <ostream>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {

class Token;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// TokenData
//
class TokenData {
 public:
  TokenData(TokenType type, hir::SimpleName* name);
  TokenData(TokenType type, uint64_t u64);
  explicit TokenData(TokenType type);
  explicit TokenData(float32_t f32);
  explicit TokenData(float64_t f64);
  explicit TokenData(hir::SimpleName* name);
  explicit TokenData(base::StringPiece16* str);
  TokenData(const TokenData& other);
  ~TokenData();

  TokenData& operator=(const TokenData& other);
  bool operator==(const TokenData& other) const;
  bool operator!=(const TokenData& other) const;

  base::char16 char_data() const;
  float32_t f32_data() const;
  float64_t f64_data() const;
  int64_t int64_data() const;
  hir::SimpleName* id() const { return simple_name(); }
  bool is_contextual_keyword() const;
  bool is_keyword() const;
  bool is_literal() const;
  bool is_name() const;
  bool is_operator() const;
  bool is_type_name() const;
  int precedence() const;
  hir::SimpleName* simple_name() const;
  base::StringPiece16 string_data() const;
  TokenType type() const { return type_; }

 private:
  union Data {
    base::char16 ch;
    float32_t f32;
    float64_t f64;
    int64_t i64;
    hir::SimpleName* name;
    base::StringPiece16* str;
    uint64_t u64;
  };

  static_assert(sizeof(Data) == sizeof(uint64_t),
                "sizeof(TokenData) must equal to sizeof(uint64_t).");

  bool has_int_data() const;
  bool has_simple_name() const;
  bool has_string_data() const;

  Data data_;
  TokenType type_;
};

std::ostream& operator<<(std::ostream& ostream, const TokenData& token);

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TOKEN_DATA_H_
