// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sstream>
#include <vector>

#include "elang/compiler/translate/translator.h"

#include "base/auto_reset.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/statements.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/predefined_names.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_data.h"
#include "elang/compiler/token_type.h"
#include "elang/compiler/translate/builder.h"
#include "elang/compiler/translate/type_mapper.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Translator::BreakContext represents target blocks of |break| and
// |continue| statements. |switch| statement also specify |continue_block|
// from outer |BreakContext|.
//
struct Translator::BreakContext final {
  ir::Control* break_block;
  ir::Control* continue_block;
  const BreakContext* outer;

  BreakContext(const BreakContext* outer,
               ir::Control* break_block,
               ir::Control* continue_block)
      : break_block(break_block),
        continue_block(continue_block),
        outer(outer) {}
};

class Translator::ScopedBreakContext final {
 public:
  ScopedBreakContext(Translator* generator,
                     ir::Control* break_block,
                     ir::Control* continue_block);
  ~ScopedBreakContext();

 private:
  Translator* const translator_;
  BreakContext const context_;

  DISALLOW_COPY_AND_ASSIGN(ScopedBreakContext);
};

Translator::ScopedBreakContext::ScopedBreakContext(Translator* generator,
                                                   ir::Control* break_block,
                                                   ir::Control* continue_block)
    : translator_(generator),
      context_(translator_->break_context_, break_block, continue_block) {
  translator_->break_context_ = &context_;
}

Translator::ScopedBreakContext::~ScopedBreakContext() {
  DCHECK_EQ(translator_->break_context_, &context_);
  translator_->break_context_ = context_.outer;
}

//////////////////////////////////////////////////////////////////////
//
// Translator
//
void Translator::TranslateStatement(ast::Statement* node) {
  DCHECK(!visit_result_);
  Traverse(node);
  DCHECK(!visit_result_);
}

//
// ast::Visitor statement nodes
//
void Translator::VisitBlockStatement(ast::BlockStatement* node) {
  builder_->StartVariableScope();

  for (auto const statement : node->statements()) {
    if (!builder()->has_control()) {
      // TODO(eval1749) Since, we may have labeled statement, we should continue
      // checking |statement|.
      break;
    }
    TranslateStatement(statement);
  }

  builder_->EndVariableScope();
}

void Translator::VisitBreakStatement(ast::BreakStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  builder_->EndBlockWithJump(break_context_->break_block);
}

void Translator::VisitContinueStatement(ast::ContinueStatement* node) {
  DCHECK(node);
  DCHECK(break_context_);
  DCHECK(break_context_->continue_block);
  builder_->EndBlockWithJump(break_context_->continue_block);
}

void Translator::VisitDoStatement(ast::DoStatement* node) {
  auto const loop_block = NewLoop();
  auto const break_block = NewMerge({});
  auto const continue_block = NewMerge({});

  builder_->StartDoLoop(loop_block);
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    TranslateStatement(node->statement());
  }
  builder_->EndBlockWithJump(continue_block);

  builder_->StartMergeBlock(continue_block);
  auto const condition = TranslateBool(node->condition());
  builder_->EndLoopBlock(condition, loop_block, break_block);

  builder_->StartMergeBlock(break_block);
}

void Translator::VisitExpressionList(ast::ExpressionList* node) {
  for (auto const expression : node->expressions())
    Translate(expression);
}

void Translator::VisitExpressionStatement(ast::ExpressionStatement* node) {
  Translate(node->expression());
}

void Translator::VisitForStatement(ast::ForStatement* node) {
  auto const loop_block = NewLoop();
  auto const continue_block = builder_->NewMergeBlock();
  auto const break_block = builder_->NewMergeBlock();

  // Loop head
  TranslateStatement(node->initializer());
  auto const head_compare = Translate(node->condition());
  builder_->StartWhileLoop(head_compare, loop_block, break_block);

  // Loop body
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    TranslateStatement(node->statement());
  }
  builder_->EndBlockWithJump(continue_block);

  // Continue block
  builder_->StartMergeBlock(continue_block);
  TranslateStatement(node->step());
  auto const continue_compare = Translate(node->condition());
  builder_->EndLoopBlock(continue_compare, loop_block, break_block);

  builder_->StartMergeBlock(break_block);
}

//    for (var element : array)
//      use(element);
//
//    head:
//      ...
//      element elty* %start = %array, 0
//      length int32 %length = %array, 0
//      element elty* %end = %array, %length
//      static_cast uintptr %1 = %ptr
//      static_cast uintptr %2 = %end
//      lt %cmp = %1, %2
//      br %cmp, loop, break
//    loop:
//      phi %ptr = head: %ptr, continue: %ptr2
//      load elty %element = %array, %ptr,
//      call $"use", %element
//      br continue
//    continue:
//      static_cast uintptr %ptrint, %ptr
//      add elty* %ptrint2 = %ptrint, sizeof(elty)
//      static_cast elty* %ptr2 = %ptrint2
//      static_cast uintptr %1 = %ptr2
//      static_cast uintptr %2 = %end
//      lt %cmp = %1, %2
//      br %cmp, loop, break
//    break:
//      ...
void Translator::VisitForEachStatement(ast::ForEachStatement* node) {
  auto const array = Translate(node->enumerable());
  if (!array->output_type()->is<ir::PointerType>()) {
    Error(ErrorCode::CodeGeneratorStatementNotYetImplemented, node);
    return;
  }
  auto const array_type = array->output_type()
                              ->as<ir::PointerType>()
                              ->pointee()
                              ->as<ir::ArrayType>();
  if (!array_type) {
    Error(ErrorCode::CodeGeneratorStatementNotYetImplemented, node);
    return;
  }

  auto const loop_block = NewLoop();
  auto const continue_block = builder_->NewMergeBlock();
  auto const break_block = builder_->NewMergeBlock();

  // Loop head
  auto const pointer_variable = session()->semantic_factory()->NewVariable(
      analysis()->SemanticOf(node->variable())->as<sm::Variable>()->type(),
      sm::StorageClass::Local, node->variable()->name());

  auto const element_type = array_type->as<ir::ArrayType>()->element_type();
  auto const element_pointer_type = NewPointerType(element_type);
  auto const start_element_pointer = NewElement(array, NewInt32(0));
  auto const end_element_pointer = NewElement(array, NewLength(array, 0));

  builder_->BindVariable(pointer_variable, start_element_pointer);
  auto const head_compare =
      NewIntCmp(ir::IntCondition::UnsignedLessThan, start_element_pointer,
                end_element_pointer);
  builder_->StartWhileLoop(head_compare, loop_block, break_block);

  // Loop body
  auto const element_pointer_0 = builder_->VariableValueOf(pointer_variable);
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    auto const element_value = builder_->NewLoad(array, element_pointer_0);
    auto const variable = SemanticOf(node->variable())->as<sm::Variable>();
    DCHECK(variable);
    builder_->StartVariableScope();
    builder_->BindVariable(variable, element_value);
    TranslateStatement(node->statement());
    builder_->EndVariableScope();
  }
  builder_->EndBlockWithJump(continue_block);

  // Continue block
  builder_->StartMergeBlock(continue_block);
  auto const pointer_int = NewStaticCast(uintptr_type(), element_pointer_0);
  auto const pointer_add = NewIntAdd(pointer_int, NewSizeOf(element_type));
  auto const element_pointer_1 =
      NewStaticCast(element_pointer_type, pointer_add);
  builder_->AssignVariable(pointer_variable, element_pointer_1);
  auto const continue_compare =
      NewIntCmp(ir::IntCondition::UnsignedLessThan, element_pointer_1,
                end_element_pointer);
  builder_->EndLoopBlock(continue_compare, loop_block, break_block);

  builder_->StartMergeBlock(break_block);
}

void Translator::VisitIfStatement(ast::IfStatement* node) {
  auto const condition = TranslateBool(node->condition());
  auto const if_node = builder_->EndBlockWithBranch(condition);
  auto const merge_node = NewMerge({});

  builder_->StartIfBlock(NewIfTrue(if_node));
  TranslateStatement(node->then_statement());
  builder_->EndBlockWithJump(merge_node);

  builder_->StartIfBlock(NewIfFalse(if_node));
  if (node->else_statement())
    TranslateStatement(node->else_statement());
  builder_->EndBlockWithJump(merge_node);

  builder_->StartMergeBlock(merge_node);
}

void Translator::VisitReturnStatement(ast::ReturnStatement* node) {
  auto const value = node->value();
  builder()->EndBlockWithRet(value ? Translate(value) : void_value());
}

void Translator::VisitVarStatement(ast::VarStatement* node) {
  for (auto const declaration : node->variables()) {
    auto const variable =
        SemanticOf(declaration->variable())->as<sm::Variable>();
    DCHECK(variable);
    builder_->BindVariable(variable, Translate(declaration->value()));
  }
}

void Translator::VisitWhileStatement(ast::WhileStatement* node) {
  auto const loop_block = NewLoop();
  auto const continue_block = builder_->NewMergeBlock();
  auto const break_block = builder_->NewMergeBlock();

  // Loop head
  auto const head_compare = Translate(node->condition());
  builder_->StartWhileLoop(head_compare, loop_block, break_block);

  // Loop body
  {
    ScopedBreakContext scope(this, break_block, continue_block);
    TranslateStatement(node->statement());
  }
  builder_->EndBlockWithJump(continue_block);

  // Continue block
  builder_->StartMergeBlock(continue_block);
  auto const continue_compare = Translate(node->condition());
  builder_->EndLoopBlock(continue_compare, loop_block, break_block);

  builder_->StartMergeBlock(break_block);
}

}  // namespace compiler
}  // namespace elang
