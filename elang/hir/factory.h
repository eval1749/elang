// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_H_
#define ELANG_HIR_FACTORY_H_

#include <memory>

#include "elang/hir/instruction_factory.h"
#include "elang/hir/type_factory.h"

namespace elang {
class Zone;
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public InstructionFactory {
 public:
  Factory();
  ~Factory();

  Zone* zone() const { return zone_.get(); }

 private:
  const std::unique_ptr<Zone> zone_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
