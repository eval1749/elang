// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_INSTRUCTION_FACTORY_H_
#define ELANG_HIR_INSTRUCTION_FACTORY_H_

#include <memory>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/hir/instructions_forward.h"
#include "elang/hir/types_forward.h"

namespace elang {
class Zone;
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// InstructionFactory
//
class InstructionFactory {
 public:
  explicit InstructionFactory(Factory* factory);
  ~InstructionFactory() = default;

  TypeFactory* types() const { return type_factory_.get(); }

  // Convenience function to have 'void' value.
  VoidLiteral* GetVoid() const;

 private:
  Zone* zone() const { return zone_.get(); }

  Factory* const factory_;
  const std::unique_ptr<Zone> zone_;
  const std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(InstructionFactory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_INSTRUCTION_FACTORY_H_
