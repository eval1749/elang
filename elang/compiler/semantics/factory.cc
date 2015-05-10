// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <unordered_map>
#include <utility>

#include "elang/compiler/semantics/factory.h"

#include "base/logging.h"
#include "elang/base/zone_user.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/visitor.h"
#include "elang/compiler/token_factory.h"

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
Factory::Factory(TokenFactory* token_factory)
    : array_type_factory_(new ArrayTypeFactory(zone())),
      global_namespace_(new (zone()) Namespace(zone(), nullptr, nullptr)),
      system_namespace_(
          NewNamespace(global_namespace_, token_factory->system_token())),
      token_factory_(token_factory) {
}

Factory::~Factory() {
}

void Factory::AddMember(Semantic* container, Semantic* member) {
  auto const name = member->name();
  if (auto const clazz = container->as<Class>()) {
    DCHECK(!clazz->FindMember(name)) << *member;
    clazz->members_.insert(std::make_pair(name->atomic_string(), member));
    return;
  }
  if (auto const ns = container->as<Namespace>()) {
    DCHECK(!ns->FindMember(name)) << *member;
    ns->members_.insert(std::make_pair(name->atomic_string(), member));
    return;
  }
  NOTREACHED() << container << " " << member;
}

ArrayType* Factory::NewArrayType(sm::Type* element_type,
                                 const std::vector<int>& dimensions) {
  return array_type_factory_->NewArrayType(element_type, dimensions);
}

Class* Factory::NewClass(Semantic* outer,
                         Token* name,
                         const std::vector<Class*>& base_classes,
                         ast::Class* ast_class) {
  auto const clazz =
      new (zone()) Class(zone(), outer, name, base_classes, ast_class);
  AddMember(outer, clazz);
  return clazz;
}

Enum* Factory::NewEnum(Semantic* outer, Token* name) {
  auto const enum_type = new (zone()) Enum(zone(), outer, name);
  AddMember(outer, enum_type);
  return enum_type;
}

EnumMember* Factory::NewEnumMember(Enum* owner, Token* name, Value* value) {
  auto const member = new (zone()) EnumMember(owner, name, value);
  owner->members_.push_back(member);
  return member;
}

Value* Factory::NewInvalidValue(Type* type, Token* token) {
  return new (zone()) InvalidValue(type, token);
}

Literal* Factory::NewLiteral(Type* type, Token* token) {
  return new (zone()) Literal(type, token);
}

Method* Factory::NewMethod(MethodGroup* method_group,
                           Signature* signature,
                           ast::Method* ast_method) {
  auto const method = new (zone()) Method(method_group, signature, ast_method);
  method_group->methods_.push_back(method);
  return method;
}

MethodGroup* Factory::NewMethodGroup(Class* owner, Token* name) {
  auto const method_group = new (zone()) MethodGroup(zone(), owner, name);
  AddMember(owner, method_group);
  return method_group;
}

Namespace* Factory::NewNamespace(Namespace* outer, Token* name) {
  DCHECK(name);
  auto const ns = new (zone()) Namespace(zone(), outer, name);
  AddMember(outer, ns);
  return ns;
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

UndefinedType* Factory::NewUndefinedType(Token* token) {
  return new (zone()) UndefinedType(token);
}

Variable* Factory::NewVariable(Type* type,
                               StorageClass storage,
                               ast::NamedNode* ast_node) {
  return new (zone()) Variable(type, storage, ast_node);
}

}  // namespace sm
}  // namespace compiler
}  // namespace elang
