// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_compiler_token_h)
#define INCLUDE_elang_compiler_token_h

#include <memory>
#include <ostream>

#include "base/strings/string_piece.h"
#include "elang/base/types.h"
#include "elang/compiler/source_code_range.h"
#include "elang/compiler/token_data.h"

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {

class TokenFactory;
enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// Token
//
class Token final {
  friend class TokenFactory;

  private: TokenData const data_;
  private: const SourceCodeRange location_;

  private: Token(const SourceCodeRange& source_range,
                 const TokenData& data);
  private: ~Token();

  public: bool operator==(const Token& other) const;
  public: bool operator!=(const Token& other) const;

  public: base::char16 char_data() const { return data_.char_data(); }
  public: TokenData data() const { return data_; }
  public: float32_t f32_data() const { return data_.f32_data(); }
  public: float64_t f64_data() const { return data_.f64_data(); }
  public: int64_t int64_data() const { return data_.int64_data(); }
  public: bool is_contextual_keyword() const {
    return data_.is_contextual_keyword();
  }
  public: bool is_keyword() const { return data_.is_keyword(); }
  public: bool is_literal() const { return data_.is_literal(); }
  public: bool is_name() const { return data_.is_name(); }
  public: bool is_operator() const { return data_.is_operator(); }
  public: bool is_type_name() const { return data_.is_type_name(); }
  public: const SourceCodeRange& location() const { return location_; }
  public: int precedence() const { return data_.precedence(); }
  public: hir::SimpleName* simple_name() const { return data_.simple_name(); }
  public: base::StringPiece16 string_data() const {
    return data_.string_data();
  }
  public: TokenType type() const { return data_.type(); }

  DISALLOW_COPY_AND_ASSIGN(Token);
};

std::ostream& operator<<(std::ostream& ostream, Token* token);

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_h)

