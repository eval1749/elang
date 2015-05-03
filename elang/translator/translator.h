// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_TRANSLATOR_TRANSLATOR_H_
#define ELANG_TRANSLATOR_TRANSLATOR_H_

#include <memory>
#include <unordered_map>

#include "elang/base/zone_owner.h"
#include "elang/lir/factory_user.h"
#include "elang/lir/value.h"
#include "elang/optimizer/node_visitor.h"

namespace elang {

namespace lir {
class BasicBlock;
class Editor;
class Factory;
enum class FloatCondition;
class Function;
class Instruction;
enum class IntegerCondition;
}

namespace optimizer {
class Schedule;
class Type;
}

namespace translator {

namespace ir = optimizer;

//////////////////////////////////////////////////////////////////////
//
// Translator
//
class Translator final : public ZoneOwner,
                         public ir::NodeVisitor,
                         public lir::FactoryUser {
 public:
  Translator(lir::Factory* factory, const ir::Schedule* schedule);
  ~Translator();

  lir::Function* Run();

 private:
  lir::Editor* editor() const { return editor_.get(); }
  lir::Function* function() const;

  lir::BasicBlock* BlockOf(ir::Node* node) const;
  void Emit(lir::Instruction* instruction);
  void EmitCopy(lir::Value output, lir::Value input);
  void EmitSetValue(lir::Value output, ir::Node* node);

  // Generate literal or |ShlInstruction|.
  lir::Value EmitShl(lir::Value input, int shift_count);

  lir::Value MapInput(ir::Node* node);
  lir::Value MapOutput(ir::Node* node);
  lir::Value MapRegister(ir::Node* node);

  // Returns type template |lir::Value| based on |ir_type|.
  static lir::Value MapType(ir::Type* ir_type);

  // Returns newly created |lir::Function| based on |ir_function|.
  static lir::Function* NewFunction(lir::Factory* factor,
                                    ir::Function* ir_function);

  void PopulatePhiOperands();
  void PrepareBlocks();

  lir::Value TranslateConditional(ir::Node* node);

// ir::NodeVisitor
#define V(Name, ...) void Visit##Name(ir::Name##Node* node) final;
  FOR_EACH_OPTIMIZER_CONCRETE_NODE(V)
#undef V

  // Map |ir::Node| to |lir::BasicBlock|.
  std::unordered_map<ir::Node*, lir::BasicBlock*> block_map_;

  // Map |ir::Node| to |lir::Value|.
  std::unordered_map<ir::Node*, lir::Value> register_map_;
  std::unique_ptr<lir::Editor> const editor_;

  // A |ir::Schedule| begin translated into |lir::Function|.
  const ir::Schedule& schedule_;

  DISALLOW_COPY_AND_ASSIGN(Translator);
};

}  // namespace translator
}  // namespace elang

#endif  // ELANG_TRANSLATOR_TRANSLATOR_H_
