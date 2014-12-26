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

namespace elang {
namespace hir {
class SimpleName;
}
namespace compiler {

enum class TokenType;

//////////////////////////////////////////////////////////////////////
//
// Token
//
class Token final {
  private: union Data {
    base::char16 ch;
    float32_t f32;
    float64_t f64;
    int64_t i64;
    hir::SimpleName* name;
    base::StringPiece16* str;
    uint64_t u64;

    Data(float32_t value) : f32(value) {
    }
    Data(float64_t value) : f64(value) {
    }
    Data(uint64_t value) : u64(value) {
    }
  };
  static_assert(sizeof(Data) == sizeof(uint64_t),
                "sizeof(Data) must equal to sizeof(uint64_t).");

  private: Data data_;
  private: SourceCodeRange location_;
  private: TokenType type_;

  public: Token(const SourceCodeRange& source_range, TokenType type);
  public: Token(const SourceCodeRange& source_range, float32_t f32);
  public: Token(const SourceCodeRange& source_range, float64_t f64);
  public: Token(const SourceCodeRange& source_range, TokenType type,
                hir::SimpleName* simple_name);
  public: Token(const SourceCodeRange& source_range,
                base::StringPiece16* string);
  public: Token(const SourceCodeRange& source_range, TokenType type,
                uint64_t u64);
  public: Token(const Token& token);
  public: Token();
  public: ~Token();

  public: Token& operator=(const Token& other);

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
  public: bool is_name() const;
  public: const SourceCodeRange& location() const { return location_; }
  public: hir::SimpleName* simple_name() const;
  public: base::StringPiece16 string_data() const;
  public: TokenType type() const { return type_; }
};

std::ostream& operator<<(std::ostream& ostream, const Token& token);

}  // namespace compiler
}  // namespace elang

#endif // !defined(INCLUDE_elang_compiler_token_h)

