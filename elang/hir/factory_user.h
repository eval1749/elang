// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_USER_H_
#define ELANG_HIR_FACTORY_USER_H_

#include "elang/hir/type_factory_user.h"

namespace elang {
class Zone;
namespace hir {

class Factory;
class Value;

//////////////////////////////////////////////////////////////////////
//
// FactoryUser
//
class ELANG_HIR_EXPORT FactoryUser : public TypeFactoryUser {
 public:
  ~FactoryUser();

  Value* false_value() const;
  Factory* factory() const { return factory_; }
  Value* true_value() const;
  Value* void_value() const;
  Zone* zone() const;

 protected:
  explicit FactoryUser(Factory* factory);

 private:
  Factory* const factory_;

  DISALLOW_COPY_AND_ASSIGN(FactoryUser);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_USER_H_
