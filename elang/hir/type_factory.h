// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TYPE_FACTORY_H_
#define ELANG_HIR_TYPE_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "elang/hir/types_forward.h"

namespace elang {
class Zone;
namespace hir {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// TypeFactory
//
class TypeFactory {
 public:
  TypeFactory();
  ~TypeFactory() = default;

  Zone* zone() const { return zone_.get(); }

  StringType* GetStringType() const { return string_type_; }

#define V(Name, ...) Name##Type* Get##Name##Type() const;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V

 private:
  const std::unique_ptr<Zone> zone_;
#define V(Name, name, ...) Name##Type* const name##_type_;
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
  StringType* const string_type_;

  DISALLOW_COPY_AND_ASSIGN(TypeFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TYPE_FACTORY_H_
