// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_FACTORY_H_
#define ELANG_COMPILER_IR_FACTORY_H_

#include <vector>

#include "elang/base/zone_owner.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/ir/nodes_forward.h"

namespace elang {
namespace compiler {
namespace ir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public ZoneOwner {
 public:
  Factory();
  ~Factory();

  Class* NewClass(ast::Class* ast_class,
                  const std::vector<Class*>& base_classes);
  Enum* NewEnum(ast::Type* ast_type, const std::vector<int64_t>& values);

  Method* NewMethod(ast::Method* ast_method, Signature* signature);

  // Allocate |Parameter| for analyzer
  Parameter* NewParameter(ast::Parameter* parameter,
                          Type* type,
                          Value* default_value);

  // Allocate |Signature| for analyzer
  Signature* NewSignature(Type* return_type,
                          const std::vector<Parameter*>& parameters);

 private:
  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_FACTORY_H_
