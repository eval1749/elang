// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>
#include <type_traits>
#include <vector>

#include "elang/compiler/semantics/calculator.h"

#include "base/logging.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace sm {

namespace {

template <typename IntType>
bool IsBoundData(const TokenData& data) {
  static_assert(
      std::is_arithmetic<IntType>::value && std::is_integral<IntType>::value,
      "IntType should be an integral type.");
  if (std::is_signed<IntType>::value) {
    auto const i64 = data.int64_data();
    return i64 >= std::numeric_limits<IntType>::min() &&
           i64 <= std::numeric_limits<IntType>::max();
  }
  auto const u64 = data.uint64_data();
  return u64 <= std::numeric_limits<IntType>::max();
}

template <>
bool IsBoundData<int64_t>(const TokenData& data) {
  return true;
}
template <>
bool IsBoundData<uint64_t>(const TokenData& data) {
  return true;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Calculator::TypeProperty
//
struct Calculator::TypeProperty {
  TokenType format;   // Float32, Float64, Int64, UInt64
  TokenType type;     // Float{32, 64}, {Int,UInt}{8, 16, 32, 64}
  TokenType literal;  // {Float, Int, UInt} {32, 64}Literal
};

//////////////////////////////////////////////////////////////////////
//
// Calculator
//
Calculator::Calculator(CompilationSession* session)
    : CompilationSessionUser(session) {
}

Calculator::~Calculator() {
}

Factory* Calculator::factory() const {
  return session()->semantics_factory();
}

Value* Calculator::Add(Value* left, int right) {
  return Add(left, NewIntValue(left->type(),
                               TokenData(TokenType::Int32Literal, right)));
}

Value* Calculator::Add(Value* left_value, Value* right_value) {
  auto const type = left_value->type();
  DCHECK_EQ(type, right_value->type()) << *left_value << " " << *right_value;

  auto const property = PropertyOf(type);
  auto const left = left_value->as<Literal>()->data();
  auto const right = right_value->as<Literal>()->data();

  if (property.format == TokenType::Int64) {
    auto const sum = left->int64_data() + right->int64_data();
    auto const carry = sum ^ left->int64_data() ^ right->int64_data();
    if (carry < 0) {
      Error(ErrorCode::SemanticIntAddOverflow, left, right);
      return NewInvalidValue(type);
    }
    return NewIntValue(type, TokenData(property.literal, sum));
  }

  if (property.format == TokenType::UInt64) {
    auto const sum = left->uint64_data() + right->uint64_data();
    if (sum < left->uint64_data()) {
      Error(ErrorCode::SemanticIntAddOverflow, left, right);
      return NewInvalidValue(type);
    }
    return NewIntValue(type, TokenData(property.literal, sum));
  }

  return NewInvalidValue(type);
}

bool Calculator::IsBound(Type* type, const TokenData& data) {
  switch (PropertyOf(type).type) {
    case TokenType::Int32:
      return IsBoundData<int32_t>(data);
  }
  return false;
}

bool Calculator::IsIntType(Type* type) {
  auto const property = PropertyOf(type);
  return property.format == TokenType::Int64 ||
         property.format == TokenType::UInt64;
}

Value* Calculator::NewIntValue(Type* type, const TokenData& data) {
  if (IsBound(type, data))
    return NewValue(type, data);
  if (IsIntType(type))
    return NewInvalidValue(type);
  Error(ErrorCode::SemanticTypeNotInteger, type->token());
  return NewInvalidValue(type);
}

Value* Calculator::NewInvalidValue(Type* type) {
  DCHECK(context_) << "You should call SetContext()";
  return factory()->NewInvalidValue(type, context_);
}

Value* Calculator::NewValue(Type* type, const TokenData& data) {
  DCHECK(context_) << "You should call SetContext()";
  return factory()->NewLiteral(type,
                               session()->NewToken(context_->location(), data));
}

Value* Calculator::Unbound(Type* type, const TokenData& data) {
  DCHECK(context_) << "You should call SetContext()";
  Error(ErrorCode::SemanticIntValueUnbound, type->token(),
        session()->NewToken(context_->location(), data));
  return NewInvalidValue(type);
}

void Calculator::SetContext(Token* token) {
  DCHECK(token);
  context_ = token;
}

Type* Calculator::PredefinedTypeOf(PredefinedName name) const {
  auto const name_token = session()->PredefinedNameOf(name);
  auto const present = factory()->system_namespace()->FindMember(name_token);
  if (!present) {
    session()->AddError(ErrorCode::PredefinedNamesNameNotFound, name_token);
    return factory()->NewUndefinedType(name_token);
  }
  auto const type = present->as<Type>();
  if (!type) {
    session()->AddError(ErrorCode::PredefinedNamesNameNotClass, name_token);
    return factory()->NewUndefinedType(name_token);
  }
  return type;
}

Calculator::TypeProperty Calculator::PropertyOf(Type* type) {
  using Ty = TokenType;

  if (PredefinedTypeOf(PredefinedName::Int32) == type)
    return TypeProperty{Ty::Int64, Ty::Int32, Ty::Int32Literal};
  if (PredefinedTypeOf(PredefinedName::Int64) == type)
    return TypeProperty{Ty::Int64, Ty::Int64, Ty::Int64Literal};
  if (PredefinedTypeOf(PredefinedName::Int16) == type)
    return TypeProperty{Ty::Int64, Ty::Int16, Ty::Int32Literal};
  if (PredefinedTypeOf(PredefinedName::Int8) == type)
    return TypeProperty{Ty::Int64, Ty::Int8, Ty::Int32Literal};
  if (PredefinedTypeOf(PredefinedName::IntPtr) == type)
    return TypeProperty{Ty::Int64, Ty::Int64, Ty::Int64Literal};

  if (PredefinedTypeOf(PredefinedName::Int32) == type)
    return TypeProperty{Ty::UInt64, Ty::UInt32, Ty::UInt32Literal};
  if (PredefinedTypeOf(PredefinedName::UInt64) == type)
    return TypeProperty{Ty::UInt64, Ty::UInt64, Ty::UInt64Literal};
  if (PredefinedTypeOf(PredefinedName::UInt16) == type)
    return TypeProperty{Ty::UInt64, Ty::UInt16, Ty::UInt32Literal};
  if (PredefinedTypeOf(PredefinedName::UInt8) == type)
    return TypeProperty{Ty::UInt64, Ty::UInt8, Ty::UInt32Literal};
  if (PredefinedTypeOf(PredefinedName::UIntPtr) == type)
    return TypeProperty{Ty::UInt64, Ty::UInt64, Ty::UInt64Literal};

  return TypeProperty{Ty::Illegal, Ty::Illegal, Ty::Illegal};
}

Value* Calculator::Zero(Type* type) {
  return NewIntValue(
      type, TokenData(PropertyOf(type).literal, static_cast<uint64_t>(0)));
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
