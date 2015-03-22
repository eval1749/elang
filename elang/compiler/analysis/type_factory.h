// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_H_
#define ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_user.h"
#include "elang/compiler/analysis/type_values_forward.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
namespace compiler {
enum class PredefinedName;
namespace ast {
class Call;
class Node;
}
namespace sm {
class Type;
}
namespace ts {

//////////////////////////////////////////////////////////////////////
//
// The Type Factory
//
class Factory final : public CompilationSessionUser, public ZoneUser {
 public:
  Factory(CompilationSession* session, Zone* zone);
  ~Factory();

  Value* any_value() const { return any_value_; }
  Value* bool_value() const { return bool_value_; }
  Value* empty_value() const { return empty_value_; }

  // Numeric types
  Value* float32_value() const { return float32_value_; }
  Value* float64_value() const { return float64_value_; }
  Value* int16_value() const { return int16_value_; }
  Value* int32_value() const { return int32_value_; }
  Value* int64_value() const { return int64_value_; }
  Value* int8_value() const { return int8_value_; }
  Value* uint16_value() const { return uint16_value_; }
  Value* uint32_value() const { return uint32_value_; }
  Value* uint64_value() const { return uint64_value_; }
  Value* uint8_value() const { return uint8_value_; }

  AndValue* NewAndValue(const std::vector<UnionValue*>& union_values);
  Argument* NewArgument(CallValue* call_value, int position);
  CallValue* NewCallValue(ast::Call* call);
  Value* NewInvalidValue(ast::Node* node);
  Value* NewLiteral(sm::Type* type);
  Value* NewNullValue(Value* value);
  Value* NewPredefinedValue(PredefinedName name);
  Variable* NewVariable(ast::Node* node, Value* value);

 private:
  Value* const any_value_;
  Value* const empty_value_;

  ZoneUnorderedMap<sm::Type*, ts::Literal*> literal_cache_map_;
  ZoneUnorderedMap<ts::Value*, ts::NullValue*> null_value_cache_map_;

  // |bool_type_| uses |literal_cache_map_|.
  Value* const bool_value_;
  Value* const float32_value_;
  Value* const float64_value_;
  Value* const int16_value_;
  Value* const int32_value_;
  Value* const int64_value_;
  Value* const int8_value_;
  Value* const uint16_value_;
  Value* const uint32_value_;
  Value* const uint64_value_;
  Value* const uint8_value_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace ts
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYSIS_TYPE_FACTORY_H_
