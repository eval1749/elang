// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !defined(INCLUDE_elang_hir_factory_h)
#define INCLUDE_elang_hir_factory_h

#include <memory>
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
  private: Namespace* const global_namespace_;
  private: std::vector<Node*> nodes_;
  private: std::unordered_map<base::StringPiece16, SimpleName*> simple_names_;
  private: std::vector<base::string16*> strings_;

  public: Factory();
  public: ~Factory();

  public: Namespace* global_namespace() const { return global_namespace_; }

  public: SimpleName* GetOrCreateSimpleName(base::StringPiece16 string);
  public: Class* NewClass(Namespace* outer, SimpleName* simple_name,
                          const std::vector<Class*>& base_classes);
  public: Namespace* NewNamespace(Namespace* outer, SimpleName* simple_name);
  public: base::StringPiece16 NewString(base::StringPiece16 string);
  public: void RemoveAll();

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace hir
}  // namespace elang

#endif // !defined(INCLUDE_elang_hir_factory_h)

