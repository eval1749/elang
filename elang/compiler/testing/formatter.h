// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_TESTING_FORMATTER_H_
#define ELANG_COMPILER_TESTING_FORMATTER_H_

#include <memory>
#include <sstream>
#include <string>

#include "base/strings/string_piece.h"
#include "elang/compiler/ast/visitor.h"
#include "elang/compiler/testing/formatter.h"

namespace elang {
namespace compiler {
namespace testing {

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public ast::Visitor {
 public:
  Formatter();
  ~Formatter() = default;

  std::string Run(ast::Node* node);

 private:
  enum class NewlineAtEnd {
    No,
    Yes,
  };

  class FormatBlock {
   public:
    FormatBlock(Formatter* formatter,
                NewlineAtEnd newline_at_end = NewlineAtEnd::Yes);
    ~FormatBlock();

   private:
    Formatter* const formatter_;
    NewlineAtEnd const newline_at_end_;

    DISALLOW_COPY_AND_ASSIGN(FormatBlock);
  };
  friend class FormatBlock;

  void FormatChildStatement(ast::Statement* statement);
  void Indent();
  void IndentPlusOne();
  void Visit(ast::Node* node);

// ast::Visitor
#define DECLARE_VISIT(type) void Visit##type(ast::type* node) final;
  FOR_EACH_CONCRETE_AST_NODE(DECLARE_VISIT)
#undef DECLARE_VISIT

  std::stringstream stream_;
  int depth_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

}  // namespace testing
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_TESTING_FORMATTER_H_
