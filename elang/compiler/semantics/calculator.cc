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
    switch (data.type()) {
      case TokenType::Int32Literal:
      case TokenType::Int64Literal:
        return data.int64_data() >=
                   static_cast<int64_t>(std::numeric_limits<IntType>::min()) &&
               data.int64_data() <=
                   static_cast<int64_t>(std::numeric_limits<IntType>::max());
      case TokenType::UInt32Literal:
      case TokenType::UInt64Literal:
        return data.uint64_data() <=
               static_cast<uint64_t>(std::numeric_limits<IntType>::max());
    }
    return false;
  }
  switch (data.type()) {
    case TokenType::Int32Literal:
    case TokenType::Int64Literal:
      return data.int64_data() >= 0 &&
             data.int64_data() <=
                 static_cast<int64_t>(std::numeric_limits<IntType>::max());
    case TokenType::UInt32Literal:
    case TokenType::UInt64Literal:
      return data.uint64_data() <=
             static_cast<uint64_t>(std::numeric_limits<IntType>::max());
  }
  return false;
}

template <>
bool IsBoundData<float32_t>(const TokenData& data) {
  if (data.is_float32())
    return true;
  if (!data.is_float64())
    return false;
  auto const f64 = static_cast<float64_t>(data.f32_data());
  return f64 >= std::numeric_limits<float32_t>::min() &&
         f64 >= std::numeric_limits<float32_t>::max();
}

template <>
bool IsBoundData<float64_t>(const TokenData& data) {
  return data.is_float64() || data.is_float32();
}

template <>
bool IsBoundData<int64_t>(const TokenData& data) {
  switch (data.type()) {
    case TokenType::Int32Literal:
    case TokenType::Int64Literal:
    case TokenType::UInt32Literal:
      return true;
    case TokenType::UInt64Literal:
      return data.uint64_data() <=
             static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
  }
  return false;
}

template <>
bool IsBoundData<uint64_t>(const TokenData& data) {
  return data.is_integer();
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

  if (left_value->is<sm::InvalidValue>())
    return left_value;
  if (right_value->is<sm::InvalidValue>())
    return right_value;

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

Value* Calculator::CastAs(Value* value, Type* type) {
  if (value->type() == type)
    return value;
  if (value->is<sm::InvalidValue>())
    return factory()->NewInvalidValue(type, value->token());
  auto const literal = value->as<sm::Literal>();
  DCHECK(literal) << value;
  return factory()->NewLiteral(type, literal->data());
}

bool Calculator::IsBound(const TokenData& data, Type* type) const {
  switch (PropertyOf(type).type) {
    case TokenType::Float32:
      return IsBoundData<float32_t>(data);
    case TokenType::Float64:
      return IsBoundData<float64_t>(data);
    case TokenType::Int16:
      return IsBoundData<int16_t>(data);
    case TokenType::Int32:
      return IsBoundData<int32_t>(data);
    case TokenType::Int64:
      return IsBoundData<int64_t>(data);
    case TokenType::Int8:
      return IsBoundData<int8_t>(data);
    case TokenType::UInt16:
      return IsBoundData<uint16_t>(data);
    case TokenType::UInt32:
      return IsBoundData<uint32_t>(data);
    case TokenType::UInt64:
      return IsBoundData<uint64_t>(data);
    case TokenType::UInt8:
      return IsBoundData<uint8_t>(data);
  }
  return false;
}

bool Calculator::IsIntType(Type* type) const {
  auto const property = PropertyOf(type);
  return property.format == TokenType::Int64 ||
         property.format == TokenType::UInt64;
}

bool Calculator::IsTypeOf(const TokenData& data, Type* type) const {
  auto const property = PropertyOf(type);
  switch (property.format) {
    case TokenType::Float32:
    case TokenType::Float64:
    case TokenType::Int64:
    case TokenType::UInt64:
      return IsBound(data, type);
  }
  return false;
}

bool Calculator::IsTypeOf(sm::Value* value, sm::Type* type) const {
  if (value->type() == type)
    return true;
  auto const literal = value->as<sm::Literal>();
  if (!literal)
    return false;
  return IsTypeOf(literal->token()->data(), type);
}

Value* Calculator::NewIntValue(Type* type, const TokenData& data) {
  DCHECK(data.is_integer()) << data;
  if (IsTypeOf(data, type))
    return NewValue(type, data);
  Error(ErrorCode::SemanticValueType, NewToken(data), type->token());
  return NewInvalidValue(type);
}

Value* Calculator::NewInvalidValue(Type* type) {
  DCHECK(context_) << "You should call SetContext()";
  return factory()->NewInvalidValue(type, context_);
}

Value* Calculator::NewValue(Type* type, const TokenData& data) {
  return factory()->NewLiteral(type, NewToken(data));
}

Token* Calculator::NewToken(const TokenData& data) {
  DCHECK(context_) << "You should call SetContext()";
  return session()->NewToken(context_->location(), data);
}

void Calculator::SetContext(Token* token) {
  DCHECK(token);
  context_ = token;
}

Type* Calculator::PredefinedTypeOf(PredefinedName name) const {
  return session()->PredefinedTypeOf(name);
}

Calculator::TypeProperty Calculator::PropertyOf(Type* type) const {
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
