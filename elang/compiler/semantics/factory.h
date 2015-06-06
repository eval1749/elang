// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_SEMANTICS_FACTORY_H_
#define ELANG_COMPILER_SEMANTICS_FACTORY_H_

#include <memory>
#include <unordered_map>
#include <vector>

#include "elang/base/zone_owner.h"
#include "elang/compiler/semantics/nodes_forward.h"

namespace elang {
namespace compiler {
class Analysis;
class Modifiers;
enum class ParameterKind;
class Token;
class TokenFactory;
namespace sm {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final : public ZoneOwner {
 public:
  explicit Factory(TokenFactory* token_factory);
  ~Factory();

  Namespace* global_namespace() { return global_namespace_; }
  Namespace* system_namespace() const { return system_namespace_; }

  // |dimensions| of each rank. dimensions.front() == -1 means unbound array.
  // Note: it is valid that one of dimension is zero. In this case, number of
  // elements in zero.
  ArrayType* NewArrayType(Type* element_type,
                          const std::vector<int>& dimensions);

  Class* NewClass(Semantic* outer, Modifiers modifiers, Token* name);
  Const* NewConst(Class* owner, Token* name);
  Enum* NewEnum(Semantic* outer, Token* name);
  EnumMember* NewEnumMember(Enum* enum_type, Token* name);
  Field* NewField(Class* owner, Modifiers modifiers, Token* name);
  Class* NewInterface(Semantic* outer, Modifiers modifiers, Token* name);
  Value* NewInvalidValue(Type* type, Token* token);
  Literal* NewLiteral(Type* type, Token* token);
  Method* NewMethod(MethodGroup* method_group,
                    Modifiers modifiers,
                    Signature* method_signature);
  MethodGroup* NewMethodGroup(Class* owner, Token* name);
  Namespace* NewNamespace(Namespace* outer, Token* name);

  // Allocate |Parameter| for analyzer
  Parameter* NewParameter(ParameterKind kind,
                          int position,
                          Type* type,
                          Token* name,
                          Value* default_value);
  PointerType* NewPointerType(Type* pointee);

  // Allocate |Signature| for analyzer
  Signature* NewSignature(Type* return_type,
                          const std::vector<Parameter*>& parameters);
  Class* NewStruct(Semantic* outer, Modifiers modifiers, Token* name);
  UndefinedType* NewUndefinedType(Token* token);
  Variable* NewVariable(Type* type, StorageClass storage, Token* name);

 private:
  class ArrayTypeFactory;

  void AddMember(Semantic* ns, Semantic* member);
  Signature* CalculateMethodFunctionSignature(Class* clazz,
                                              Modifiers modifiers,
                                              Signature* method_signature);

  std::unique_ptr<ArrayTypeFactory> array_type_factory_;
  Namespace* const global_namespace_;
  std::unordered_map<Type*, PointerType*> pointer_types_;
  Namespace* const system_namespace_;
  TokenFactory* const token_factory_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace sm
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_SEMANTICS_FACTORY_H_
