// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/ast/types.h"

#include "base/logging.h"

#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace ast {

//////////////////////////////////////////////////////////////////////
//
// Types
//

// Type
Type::Type(Token* token) : Expression(token) {
}

// ArrayType
ArrayType::ArrayType(Zone* zone,
                     Token* op,
                     Type* element_type,
                     const std::vector<int>& dimensions)
    : Type(op), dimensions_(zone, dimensions), element_type_(element_type) {
  DCHECK(!dimensions_.empty());
#ifndef NDEBUG
  for (auto const dimension : dimensions_)
    DCHECK_GE(dimension, -1);
#endif
}

// ConstructedType
ConstructedType::ConstructedType(ConstructedName* reference)
    : Type(reference->token()), reference_(reference) {
}

// InvalidType
InvalidType::InvalidType(Expression* expression)
    : Type(expression->token()), expression_(expression) {
}

// OptionalType
OptionalType::OptionalType(Token* op, Type* base_type)
    : Type(op), base_type_(base_type) {
  DCHECK_EQ(op, TokenType::OptionalType);
}

// TypeMemberAccess
TypeMemberAccess::TypeMemberAccess(MemberAccess* reference)
    : Type(reference->token()), reference_(reference) {
}

// TypeNameReference
TypeNameReference::TypeNameReference(NameReference* reference)
    : Type(reference->token()), reference_(reference) {
}

Token* TypeNameReference::name() const {
  return reference_->name();
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
