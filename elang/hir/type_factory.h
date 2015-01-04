// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPE_FACTORY_H_
#define ELANG_HIR_TYPE_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "elang/hir/hir_export.h"
#include "elang/hir/types_forward.h"

namespace elang {
class Zone;
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
class ELANG_HIR_EXPORT TypeFactory {
 public:
  TypeFactory();
  ~TypeFactory();

  Zone* zone() const { return zone_.get(); }

  StringType* GetStringType() const { return string_type_; }
  FunctionType* NewFunctionType(Type* return_type, Type* parameters_type);

#define V(Name, ...) Name##Type* Get##Name##Type() const;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

 private:
  class FunctionTypeFactory;

  const std::unique_ptr<Zone> zone_;

#define V(Name, name, ...) Name##Type* const name##_type_;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

  std::unique_ptr<FunctionTypeFactory> function_type_factory_;
  StringType* const string_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPE_FACTORY_H_
