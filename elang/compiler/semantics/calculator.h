// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_CALCULATOR_H_
#define ELANG_COMPILER_SEMANTICS_CALCULATOR_H_

#include <cstdint>

#include "base/macros.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
enum class PredefinedName;
class TokenData;
enum class TokenType;
namespace sm {
class Factory;
class Type;
class Value;

//////////////////////////////////////////////////////////////////////
//
// Calculator
//
class Calculator final : public CompilationSessionUser {
 public:
  explicit Calculator(CompilationSession* session);
  ~Calculator();

  Value* Add(Value* left, int right);
  Value* Add(Value* left, Value* right);
  bool IsIntType(Type* type);
  Value* NewIntValue(Type* type, const TokenData& data);
  void SetContext(Token* token);
  Value* Zero(Type* type);

 private:
  struct TypeProperty;

  Factory* factory() const;

  bool IsBound(Type* type, const TokenData& data);
  Value* NewInvalidValue(Type* type);
  Value* NewValue(Type* type, const TokenData& data);
  Type* PredefinedTypeOf(PredefinedName name) const;
  TypeProperty PropertyOf(Type* type);
  Value* Unbound(Type* type, const TokenData& data);

  Token* context_;

  DISALLOW_COPY_AND_ASSIGN(Calculator);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_CALCULATOR_H_
