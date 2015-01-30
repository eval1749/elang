// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_USER_H_
#define ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_USER_H_

#include "base/macros.h"

namespace elang {
namespace compiler {

namespace ast {
class Node;
}

namespace ir {
class Type;
}

namespace ts {

class Factory;
class Value;

//////////////////////////////////////////////////////////////////////
//
// FactoryUser
//
class FactoryUser {
 public:
  ~FactoryUser();

  Factory* factory() const { return factory_; }
  Factory* type_factory() const { return factory_; }

 protected:
  explicit FactoryUser(Factory* factory);

  Value* any_value() const;
  Value* bool_value() const;
  Value* empty_value() const;
  Value* float32_value() const;
  Value* float64_value() const;
  Value* int16_value() const;
  Value* int32_value() const;
  Value* int64_value() const;
  Value* int8_value() const;
  Value* uint16_value() const;
  Value* uint32_value() const;
  Value* uint64_value() const;
  Value* uint8_value() const;

  Value* NewInvalidValue(ast::Node* node);
  Value* NewLiteral(ir::Type* type);

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(FactoryUser);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_USER_H_
