// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/logging.h"
#include "base/strings/string_piece.h"
#include "base/macros.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

namespace {

TEST(TokenTest, LiteralType) {
  base::StringPiece16 string(L"foo");
  AtomicStringFactory factory;
  auto const dummy = factory.NewAtomicString(L"dummy");

  EXPECT_EQ(PredefinedName::Char,
            TokenData(TokenType::CharacterLiteral, 'a').literal_type());
  EXPECT_EQ(PredefinedName::Bool,
            TokenData(TokenType::FalseLiteral, dummy).literal_type());
  EXPECT_EQ(PredefinedName::Float32, TokenData(1.23f).literal_type());
  EXPECT_EQ(PredefinedName::Float64, TokenData(1.23).literal_type());
  EXPECT_EQ(PredefinedName::String, TokenData(&string).literal_type());
  EXPECT_EQ(PredefinedName::Int32,
            TokenData(TokenType::Int32Literal, 0x10000).literal_type());
  EXPECT_EQ(PredefinedName::Int64,
            TokenData(TokenType::Int64Literal, 0x10000).literal_type());
  EXPECT_EQ(PredefinedName::UInt32,
            TokenData(TokenType::UInt32Literal, 0x10000).literal_type());
  EXPECT_EQ(PredefinedName::UInt64,
            TokenData(TokenType::UInt64Literal, 0x10000).literal_type());
  EXPECT_EQ(PredefinedName::Bool,
            TokenData(TokenType::TrueLiteral, dummy).literal_type());
}

TEST(TokenTest, TypeKeyword) {
  AtomicStringFactory factory;
  auto const dummy = factory.NewAtomicString(L"dummy");
#define Int Int32
#define V(Name)                                                  \
  EXPECT_TRUE(TokenData(TokenType::Name, dummy).is_type_name()); \
  EXPECT_EQ(PredefinedName::Name,                                \
            TokenData(TokenType::Name, dummy).mapped_type_name());
  FOR_EACH_TYPE_KEYWORD(V)
#undef V
#undef Int
  EXPECT_TRUE(TokenData(TokenType::Int, dummy).is_type_name());
  EXPECT_EQ(PredefinedName::Int32,
            TokenData(TokenType::Int, dummy).mapped_type_name());
}

}  // namespace
}  // namespace compiler
}  // namespace elang
