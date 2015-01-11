// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
namespace compiler {

namespace {

TEST(TokenTest, TypeKeyword) {
  AtomicStringFactory factory;
  auto const dummy = factory.NewAtomicString(L"dummy");
#define Int Int32
#define V(Name)                                           \
  EXPECT_TRUE(TokenData(TokenType::Name, dummy).is_type_name()); \
  EXPECT_EQ(PredefinedName::Name,                         \
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
