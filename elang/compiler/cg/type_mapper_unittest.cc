// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/cg/cg_test.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/cg/type_mapper.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/hir/factory.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/types.h"

namespace elang {
namespace compiler {
namespace {

//////////////////////////////////////////////////////////////////////
//
// TypeMapperTest
//
class TypeMapperTest : public testing::CgTest {
 protected:
  TypeMapperTest();
  ~TypeMapperTest() override = default;

  sm::Factory* ir_factory() { return &ir_factory_; }
  hir::TypeFactory* types() { return factory()->types(); }
  TypeMapper* type_mapper() { return &type_mapper_; }

  hir::Type* Map(PredefinedName name);
  hir::Type* Map(sm::Type* type);

  // IR type constructors
  sm::Type* GetIr(PredefinedName name);
  sm::Type* GetIr(sm::Type* type) { return type; }
  sm::Parameter* NewParameter(sm::Type* type);
  sm::Parameter* NewParameter(PredefinedName name);

  template <typename... T>
  std::vector<sm::Parameter*> NewParameters(T... params) {
    return {NewParameter(params)...};
  }
  std::vector<sm::Parameter*> NewParameters() { return {}; }

  template <typename R, typename... T>
  sm::Signature* NewSignature(R return_type, T... params) {
    return ir_factory()->NewSignature(GetIr(return_type),
                                      NewParameters(params...));
  }

 private:
  sm::Factory ir_factory_;
  testing::NamespaceBuilder builder_;
  TypeMapper type_mapper_;

  DISALLOW_COPY_AND_ASSIGN(TypeMapperTest);
};

TypeMapperTest::TypeMapperTest()
    : builder_(name_resolver()), type_mapper_(session(), factory()) {
}

sm::Type* TypeMapperTest::GetIr(PredefinedName name) {
  return semantics()
      ->SemanticOf(session()->PredefinedTypeOf(name))
      ->as<sm::Type>();
}

hir::Type* TypeMapperTest::Map(PredefinedName name) {
  return Map(GetIr(name));
}

hir::Type* TypeMapperTest::Map(sm::Type* type) {
  return type_mapper()->Map(type);
}

sm::Parameter* TypeMapperTest::NewParameter(sm::Type* type) {
  auto const ast_type = session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(builder_.NewName("type")));
  auto const ast_parameter = session()->ast_factory()->NewParameter(
      nullptr, ParameterKind::Required, 0, ast_type, builder_.NewName("param"),
      nullptr);
  return ir_factory()->NewParameter(ast_parameter, type, nullptr);
}

sm::Parameter* TypeMapperTest::NewParameter(PredefinedName name) {
  return NewParameter(GetIr(name));
}

//////////////////////////////////////////////////////////////////////
//
// Test cases...
//

TEST_F(TypeMapperTest, ArrayType) {
  std::vector<int> dimensions{-1};
  EXPECT_EQ(types()->NewPointerType(
                types()->NewArrayType(types()->int32_type(), dimensions)),
            Map(ir_factory()->NewArrayType(GetIr(PredefinedName::Int32),
                                           dimensions)));
}

TEST_F(TypeMapperTest, FunctionType) {
  EXPECT_EQ(
      types()->NewFunctionType(types()->void_type(), types()->void_type()),
      Map(NewSignature(PredefinedName::Void, PredefinedName::Void)));
  EXPECT_EQ(
      types()->NewFunctionType(types()->int32_type(), types()->float32_type()),
      Map(NewSignature(PredefinedName::Int32, PredefinedName::Float32)));
}

TEST_F(TypeMapperTest, PrimitiveTypes) {
#define V(Name, name, ...) \
  EXPECT_EQ(types()->name##_type(), Map(GetIr(PredefinedName::Name)));
  FOR_EACH_HIR_PRIMITIVE_TYPE(V)
#undef V
}

}  // namespace
}  // namespace compiler
}  // namespace elang
