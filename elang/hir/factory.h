// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_factory_h)
#define INCLUDE_elang_hir_factory_h

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"

namespace elang {
namespace hir {
class Class;
class Enum;
class EnumMember;
class Expression;
class Namespace;
class Node;
class SimpleName;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final {
 public:
  Factory();
  ~Factory();

  Namespace* global_namespace() const { return global_namespace_; }

  SimpleName* GetOrCreateSimpleName(base::StringPiece16 string);
  Class* NewClass(Namespace* outer, SimpleName* simple_name,
                  const std::vector<Class*>& base_classes);
  Namespace* NewNamespace(Namespace* outer, SimpleName* simple_name);
  base::StringPiece16 NewString(base::StringPiece16 string);
  SimpleName* NewUniqueName(const base::char16* format);
  void RemoveAll();

 private:
  std::vector<Node*> nodes_;
  std::unordered_map<base::StringPiece16, SimpleName*> simple_names_;
  std::vector<base::string16*> strings_;
  int temp_name_counter_;

  // |global_namespace_| must be initialized after node pool and string pool
  // are created.
  Namespace* const global_namespace_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif  // !defined(INCLUDE_elang_hir_factory_h)

