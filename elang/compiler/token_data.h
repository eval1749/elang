// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_token_data_h)
#define INCLUDE_elang_compiler_token_data_h

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
  private: union Data {
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

  private: Data data_;
  private: TokenType type_;

  public: TokenData(TokenType type, hir::SimpleName* name);
  public: TokenData(TokenType type, uint64_t u64);
  public: explicit TokenData(TokenType type);
  public: explicit TokenData(float32_t f32);
  public: explicit TokenData(float64_t f64);
  public: explicit TokenData(hir::SimpleName* name);
  public: explicit TokenData(base::StringPiece16* str);
  public: TokenData(const TokenData& other);
  public: ~TokenData();

  public: TokenData& operator=(const TokenData& other);
  public: bool operator==(const TokenData& other) const;
  public: bool operator!=(const TokenData& other) const;

  public: base::char16 char_data() const;
  private: bool has_int_data() const;
  private: bool has_simple_name() const;
  private: bool has_string_data() const;
  public: float32_t f32_data() const;
  public: float64_t f64_data() const;
  public: int64_t int64_data() const;
  public: hir::SimpleName* id() const { return simple_name(); }
  public: bool is_contextual_keyword() const;
  public: bool is_keyword() const;
  public: bool is_literal() const;
  public: bool is_name() const;
  public: bool is_operator() const;
  public: int precedence() const;
  public: hir::SimpleName* simple_name() const;
  public: base::StringPiece16 string_data() const;
  public: TokenType type() const { return type_; }
};

std::ostream& operator<<(std::ostream& ostream, const TokenData& token);

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_data_h)

