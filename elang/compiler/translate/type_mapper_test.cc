// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/translate/translate_test.h"

#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/parameter_kind.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/semantics/semantics.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/compiler/translate/type_mapper.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/type_factory.h"
#include "elang/optimizer/types.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// IrTypeMapperTest
//
class IrTypeMapperTest : public testing::TranslateTest {
 protected:
  IrTypeMapperTest();
  ~IrTypeMapperTest() override = default;

  sm::Factory* semantics_factory() { return &semantics_factory_; }
  ir::TypeFactory* types() { return factory()->type_factory(); }
  IrTypeMapper* type_mapper() { return &type_mapper_; }

  ir::Type* Map(cm::PredefinedName name);
  ir::Type* Map(sm::Type* type);

  // IR type constructors
  sm::Type* GetIr(cm::PredefinedName name);
  sm::Type* GetIr(sm::Type* type) { return type; }
  sm::Parameter* NewParameter(sm::Type* type);
  sm::Parameter* NewParameter(cm::PredefinedName name);

  template <typename... T>
  std::vector<sm::Parameter*> NewParameters(T... params) {
    return {NewParameter(params)...};
  }
  std::vector<sm::Parameter*> NewParameters() { return {}; }

  template <typename R, typename... T>
  sm::Signature* NewSignature(R return_type, T... params) {
    return semantics_factory()->NewSignature(GetIr(return_type),
                                             NewParameters(params...));
  }

 private:
  sm::Factory semantics_factory_;
  cm::testing::NamespaceBuilder builder_;
  IrTypeMapper type_mapper_;

  DISALLOW_COPY_AND_ASSIGN(IrTypeMapperTest);
};

IrTypeMapperTest::IrTypeMapperTest()
    : builder_(name_resolver()),
      type_mapper_(session(), factory()->type_factory()) {
}

sm::Type* IrTypeMapperTest::GetIr(cm::PredefinedName name) {
  return semantics()
      ->SemanticOf(session()->PredefinedTypeOf(name))
      ->as<sm::Type>();
}

ir::Type* IrTypeMapperTest::Map(cm::PredefinedName name) {
  return Map(GetIr(name));
}

ir::Type* IrTypeMapperTest::Map(sm::Type* type) {
  return type_mapper()->Map(type);
}

sm::Parameter* IrTypeMapperTest::NewParameter(sm::Type* type) {
  auto const ast_type = session()->ast_factory()->NewTypeNameReference(
      session()->ast_factory()->NewNameReference(builder_.NewName("type")));
  auto const ast_parameter = session()->ast_factory()->NewParameter(
      nullptr, cm::ParameterKind::Required, 0, ast_type,
      builder_.NewName("param"), nullptr);
  return semantics_factory()->NewParameter(ast_parameter, type, nullptr);
}

sm::Parameter* IrTypeMapperTest::NewParameter(cm::PredefinedName name) {
  return NewParameter(GetIr(name));
}

TEST_F(IrTypeMapperTest, ArrayType) {
  std::vector<int> dimensions{-1};
  EXPECT_EQ(types()->NewPointerType(
                types()->NewArrayType(types()->int32_type(), dimensions)),
            Map(semantics_factory()->NewArrayType(
                GetIr(cm::PredefinedName::Int32), dimensions)));
}

TEST_F(IrTypeMapperTest, FunctionType) {
  EXPECT_EQ(
      types()->NewFunctionType(types()->void_type(), types()->void_type()),
      Map(NewSignature(cm::PredefinedName::Void, cm::PredefinedName::Void)));
  EXPECT_EQ(
      types()->NewFunctionType(types()->int32_type(), types()->float32_type()),
      Map(NewSignature(cm::PredefinedName::Int32,
                       cm::PredefinedName::Float32)));
}

TEST_F(IrTypeMapperTest, PrimitiveTypes) {
#define V(Name, name, ...) \
  EXPECT_EQ(types()->name##_type(), Map(GetIr(cm::PredefinedName::Name)));
  FOR_EACH_OPTIMIZER_PRIMITIVE_TYPE(V)
#undef V
}

}  // namespace compiler
}  // namespace elang
