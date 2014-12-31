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

namespace elang {
class Zone;
namespace hir {
class Class;
class Enum;
class EnumMember;
class Expression;
class Namespace;
class Node;
class AtomicString;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final {
 public:
  Factory();
  ~Factory();

  Namespace* global_namespace() const { return global_namespace_; }

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
  std::vector<base::string16*> strings_;
  int temp_name_counter_;

  // |global_namespace_| must be initialized after |zone_|.
  Namespace* const global_namespace_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_H_
