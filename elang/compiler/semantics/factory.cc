// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <utility>

#include "elang/compiler/semantics/factory.h"

#include "base/logging.h"
#include "elang/base/zone_user.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"
#include "elang/compiler/semantics/visitor.h"

namespace {
using elang::compiler::sm::Type;
typedef std::pair<Type*, std::vector<int>> ArrayProperty;
}

namespace std {
template <>
struct hash<ArrayProperty> {
  size_t operator()(const ArrayProperty& pair) const {
    auto hash_code = std::hash<Type*>()(pair.first);
    for (auto const dimension : pair.second) {
      auto const upper = hash_code >> (sizeof(hash_code) - 3);
      hash_code <<= 3;
      hash_code ^= std::hash<int>()(dimension);
      hash_code ^= upper;
    }
    return hash_code;
  }
};
}  // namespace std

namespace elang {
namespace compiler {
namespace sm {

// Implementation of "visitor" pattern.
#define V(Name) \
  void Name::Accept(Visitor* visitor) { visitor->Visit##Name(this); }
FOR_EACH_CONCRETE_SEMANTIC(V)
#undef V

//////////////////////////////////////////////////////////////////////
//
// Factory::ArrayTypeFactory
//
class Factory::ArrayTypeFactory final : public ZoneUser {
 public:
  explicit ArrayTypeFactory(Zone* zone) : ZoneUser(zone) {}
  ~ArrayTypeFactory() = default;

  ArrayType* NewArrayType(Type* element_type,
                          const std::vector<int>& dimensions);

 private:
  std::unordered_map<ArrayProperty, ArrayType*> map_;

  DISALLOW_COPY_AND_ASSIGN(ArrayTypeFactory);
};

ArrayType* Factory::ArrayTypeFactory::NewArrayType(
    Type* element_type,
    const std::vector<int>& dimensions) {
  const auto key = std::make_pair(element_type, dimensions);
  const auto it = map_.find(key);
  if (it != map_.end())
    return it->second;
  auto const new_type =
      new (zone()) ArrayType(zone(), element_type, dimensions);
  map_[key] = new_type;
  return new_type;
}

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : array_type_factory_(new ArrayTypeFactory(zone())),
      semantics_(new Semantics()) {
}

Factory::~Factory() {
}

ArrayType* Factory::NewArrayType(sm::Type* element_type,
                                 const std::vector<int>& dimensions) {
  return array_type_factory_->NewArrayType(element_type, dimensions);
}

Class* Factory::NewClass(ast::Class* ast_class,
                         const std::vector<Class*>& base_classes) {
  return new (zone()) Class(zone(), ast_class, base_classes);
}

Enum* Factory::NewEnum(Token* name, Type* enum_base) {
  return new (zone()) Enum(zone(), name, enum_base);
}

EnumMember* Factory::NewEnumMember(Enum* owner, Token* name) {
  DCHECK(owner->members_.empty());
  return new (zone()) EnumMember(owner, name);
}

Literal* Factory::NewLiteral(Type* type, Token* token) {
  return new (zone()) Literal(type, token);
}

Method* Factory::NewMethod(MethodGroup* method_group,
                           Signature* signature,
                           ast::Method* ast_method) {
  return new (zone()) Method(method_group, signature, ast_method);
}

MethodGroup* Factory::NewMethodGroup(Class* owner, Token* name) {
  return new (zone()) MethodGroup(zone(), owner, name);
}

Parameter* Factory::NewParameter(ast::Parameter* ast_parameter,
                                 Type* type,
                                 Value* default_value) {
  return new (zone()) Parameter(ast_parameter, type, default_value);
}

Signature* Factory::NewSignature(Type* return_type,
                                 const std::vector<Parameter*>& parameters) {
  return new (zone()) Signature(zone(), return_type, parameters);
}

UndefinedType* Factory::NewUndefinedType(ast::Type* type) {
  return new (zone()) UndefinedType(type);
}

Variable* Factory::NewVariable(Type* type,
                               StorageClass storage,
                               ast::NamedNode* ast_node) {
  return new (zone()) Variable(type, storage, ast_node);
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
