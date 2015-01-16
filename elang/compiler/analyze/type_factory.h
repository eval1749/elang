// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_
#define ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_

#include "base/macros.h"
#include "elang/base/zone_owner.h"
#include "elang/compiler/analyze/type_values_forward.h"

namespace elang {
namespace compiler {
namespace ast {
class Node;
}
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public ZoneOwner {
 public:
  Factory();
  ~Factory();

  AnyValue* GetAnyValue() { return any_value_; }
  EmptyValue* GetEmptyValue() { return empty_value_; }
  InvalidValue* NewInvalidValue(ast::Node* node);

 private:
  AnyValue* const any_value_;
  EmptyValue* const empty_value_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_
