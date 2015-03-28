// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_NODE_CACHE_H_
#define ELANG_OPTIMIZER_NODE_CACHE_H_

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_user.h"
#include "elang/optimizer/nodes_forward.h"
#include "elang/optimizer/types_forward.h"

namespace elang {
class AtomicString;
class Zone;
namespace optimizer {

template <typename... Types>
struct KeyTuple;

//////////////////////////////////////////////////////////////////////
//
// NodeCache
//
class NodeCache final : public ZoneUser {
 public:
  NodeCache(Zone* zone, TypeFactory* type_factory);
  ~NodeCache();

#define V(Name, mnemonic, data_type, ...) \
  Node* New##Name(Type* type, data_type data);
  FOR_EACH_OPTIMIZER_CONCRETE_LITERAL_NODE(V)
#undef V
  Node* NewFunctionReference(Type* output_type, Function* function);
  Node* NewNull(Type* type);
  Node* NewReference(Type* type, AtomicString* name);

 private:
#define V(Name, name, data_type, ...) \
  std::unordered_map<data_type, Node*> name##_cache_;
  FOR_EACH_OPTIMIZER_PRIMITIVE_VALUE_TYPE(V)
#undef V
  std::unordered_map<Function*, Node*> function_literal_cache_;
  std::unordered_map<Type*, Node*> null_literal_cache_;
  std::map<KeyTuple<Type*, AtomicString*>, Node*> reference_cache_;
  std::unordered_map<base::StringPiece16, Node*> string_cache_;
  TypeFactory* const type_factory_;

  DISALLOW_COPY_AND_ASSIGN(NodeCache);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_NODE_CACHE_H_
