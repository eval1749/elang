// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_H_
#define ELANG_HIR_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/hir/instruction_factory.h"
#include "elang/hir/type_factory.h"

namespace elang {
class Zone;
namespace hir {
class AtomicString;
class Class;
class Enum;
class EnumMember;
class Expression;
class Namespace;
class Node;
class TypeFactory;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public InstructionFactory {
 public:
  Factory();
  ~Factory();

  Namespace* global_namespace() const { return global_namespace_; }
  Zone* zone() const { return zone_.get(); }

  AtomicString* GetOrCreateAtomicString(base::StringPiece16 string);
  Class* NewClass(Namespace* outer,
                  AtomicString* simple_name,
                  const std::vector<Class*>& base_classes);
  Namespace* NewNamespace(Namespace* outer, AtomicString* simple_name);
  base::StringPiece16 NewString(base::StringPiece16 string);
  AtomicString* NewUniqueName(const base::char16* format);

 private:
  const std::unique_ptr<Zone> zone_;
  std::unordered_map<base::StringPiece16, AtomicString*> simple_names_;
  int temp_name_counter_;

  // |global_namespace_| must be initialized after |zone_|.
  Namespace* const global_namespace_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
