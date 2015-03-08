// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_TESTING_HIR_TEST_H_
#define ELANG_HIR_TESTING_HIR_TEST_H_

#include <memory>
#include <string>

#include "base/macros.h"
#include "elang/hir/factory_user.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;
class Zone;

namespace hir {
class Editor;
class ErrorData;
class Factory;
struct FactoryConfig;
class Function;
class Type;
class TypeFactory;

namespace testing {

//////////////////////////////////////////////////////////////////////
//
// HirTest
//
class HirTest : public ::testing::Test, public FactoryUser {
 protected:
  HirTest();
  ~HirTest() override;

  Editor* editor() { return editor_.get(); }
  BasicBlock* entry_block() const;
  BasicBlock* exit_block() const;
  Function* function() const { return function_; }

  std::string Format(Function* function);
  std::string Format();
  std::string GetErrors();
  Function* NewFunction(Type* return_type, Type* parameters_type);
  Function* NewSampleFunction();
  std::string ToString(Instruction* instruction);
  std::string ToString(Type* type);
  std::string ToString(Value* value);
  std::string Validate();

 private:
  std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  std::unique_ptr<Factory> factory_;

  // Following objects require |Factory|.
  Function* function_;
  const std::unique_ptr<Editor> editor_;

  DISALLOW_COPY_AND_ASSIGN(HirTest);
};

}  // namespace testing
}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_TESTING_HIR_TEST_H_
