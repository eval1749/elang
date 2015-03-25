// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FACTORY_USER_H_
#define ELANG_OPTIMIZER_FACTORY_USER_H_

#include "elang/optimizer/node_factory_user.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/type_factory_user.h"

namespace elang {
class Zone;
namespace optimizer {

class Factory;

//////////////////////////////////////////////////////////////////////
//
// FactoryUser
//
class ELANG_OPTIMIZER_EXPORT FactoryUser : public NodeFactoryUser,
                                           public TypeFactoryUser {
 public:
  ~FactoryUser();

  Factory* factory() const { return factory_; }
  Zone* zone() const;

  Function* NewFunction(FunctionType* function_type);

 protected:
  explicit FactoryUser(Factory* factory);

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(FactoryUser);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FACTORY_USER_H_
