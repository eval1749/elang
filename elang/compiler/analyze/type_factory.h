// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_
#define ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_user.h"
#include "elang/compiler/analyze/type_values_forward.h"

namespace elang {
namespace compiler {
namespace ast {
class Call;
class Node;
}
namespace ir {
class Type;
}
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// The Type Factory
//
class Factory final : public ZoneUser {
 public:
  explicit Factory(Zone* zone);
  ~Factory();

  AnyValue* GetAnyValue() { return any_value_; }
  EmptyValue* GetEmptyValue() { return empty_value_; }
  AndValue* NewAndValue(const std::vector<UnionValue*>& union_values);
  Argument* NewArgument(CallValue* call_value, int position);
  CallValue* NewCallValue(ast::Call* call);
  InvalidValue* NewInvalidValue(ast::Node* node);
  Literal* NewLiteral(ir::Type* type);
  NullValue* NewNullValue(Value* value);
  Variable* NewVariable(ast::Node* node, Value* value);

 private:
  AnyValue* const any_value_;
  EmptyValue* const empty_value_;

  ZoneUnorderedMap<ir::Type*, ts::Literal*> literal_cache_map_;
  ZoneUnorderedMap<ts::Value*, ts::NullValue*> null_value_cache_map_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_TYPE_FACTORY_H_
