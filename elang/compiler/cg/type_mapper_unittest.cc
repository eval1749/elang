// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/cg/cg_test.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/cg/type_mapper.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics.h"
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

  ir::Factory* ir_factory() { return &ir_factory_; }
  hir::TypeFactory* types() { return factory()->types(); }
  TypeMapper* type_mapper() { return &type_mapper_; }

  hir::Type* Map(PredefinedName name);
  hir::Type* Map(ir::Type* type);

  // IR type constructors
  ir::Type* GetIr(PredefinedName name);
  ir::Type* GetIr(ir::Type* type) { return type; }
  ir::Parameter* NewParameter(ir::Type* type);
  ir::Parameter* NewParameter(PredefinedName name);

  template <typename... T>
  std::vector<ir::Parameter*> NewParameters(T... params) {
    return {NewParameter(params)...};
  }
  std::vector<ir::Parameter*> NewParameters() { return {}; }

  template <typename R, typename... T>
  ir::Signature* NewSignature(R return_type, T... params) {
    return ir_factory()->NewSignature(GetIr(return_type),
                                      NewParameters(params...));
  }

 private:
  ir::Factory ir_factory_;
  testing::NamespaceBuilder builder_;
  TypeMapper type_mapper_;

  DISALLOW_COPY_AND_ASSIGN(TypeMapperTest);
};

TypeMapperTest::TypeMapperTest()
    : builder_(name_resolver()), type_mapper_(session(), factory()) {
}

ir::Type* TypeMapperTest::GetIr(PredefinedName name) {
  return semantics()
      ->ValueOf(session()->GetPredefinedType(name))
      ->as<ir::Type>();
}

hir::Type* TypeMapperTest::Map(PredefinedName name) {
  return Map(GetIr(name));
}

hir::Type* TypeMapperTest::Map(ir::Type* type) {
  return type_mapper()->Map(type);
}

ir::Parameter* TypeMapperTest::NewParameter(ir::Type* type) {
  auto const ast_type = session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(builder_.NewName("type")));
  auto const ast_parameter = session()->ast_factory()->NewParameter(
      nullptr, ParameterKind::Required, 0, ast_type, builder_.NewName("param"),
      nullptr);
  return ir_factory()->NewParameter(ast_parameter, type, nullptr);
}

ir::Parameter* TypeMapperTest::NewParameter(PredefinedName name) {
  return NewParameter(GetIr(name));
}

// Tests...

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
