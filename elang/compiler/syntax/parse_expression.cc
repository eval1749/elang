// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/assignment.h"
#include "elang/compiler/ast/binary_operation.h"
#include "elang/compiler/ast/call.h"
#include "elang/compiler/ast/conditional.h"
#include "elang/compiler/ast/constructed_type.h"
#include "elang/compiler/ast/literal.h"
#include "elang/compiler/ast/name_reference.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/ast/unary_operation.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// Parser::ExpressionCategory
//
#define EXPRESSION_CATEGORY_LIST(V)         \
  V(None)                                   \
  V(Primary)                                \
  V(Unary)          /* '++' '--' '~' '!' */ \
  V(Multiplicative) /* '*' '/' /%' */       \
  V(Additive)       /* '+' '-' */           \
  V(Shift)          /* '<<' '>>' */         \
  V(Relational)     /* '<' '<=' '>' '>=' */ \
  V(Equality)       /* '==') '!=' */        \
  V(BitAnd)         /* '&' */               \
  V(BitXor)         /* '^' */               \
  V(BitOr)          /* '|' */               \
  V(ConditionalAnd) /* '&&' */              \
  V(ConditionalOr)  /* '||' */              \
  V(NullCoalescing) /* '??' */              \
  V(Conditional)    /* '?:' */              \
  V(Assignment)     /* '=' '+=' ... */

enum class Parser::ExpressionCategory {
#define ENUM_MEMBER(name) name,
  EXPRESSION_CATEGORY_LIST(ENUM_MEMBER)
#undef ENUM_MEMBER
};

std::ostream& operator<<(std::ostream& ostream,
                         Parser::ExpressionCategory category) {
  static const char* const strings[] = {
#define V(name) #name,
      EXPRESSION_CATEGORY_LIST(V)
#undef V
  };
  return ostream << strings[static_cast<int>(category)];
}

namespace {
Parser::ExpressionCategory RaisePrecedence(
    Parser::ExpressionCategory category) {
  DCHECK_NE(category, Parser::ExpressionCategory::None);
  DCHECK_NE(category, Parser::ExpressionCategory::Primary);
  return static_cast<Parser::ExpressionCategory>(static_cast<int>(category) -
                                                 1);
}
}  // namespace

ast::Expression* Parser::ConsumeExpression() {
  DCHECK(expression_);
  auto const result = expression_;
  expression_ = nullptr;
  return result;
}

Token* Parser::ConsumeTokenAs(TokenType type) {
  auto const original = ConsumeToken();
  return session_->NewToken(original->location(), TokenData(type));
}

bool Parser::ParseExpression() {
  // Expression ::= ConditionalExpression | Assignment
  if (!ParseExpressionSub(ExpressionCategory::NullCoalescing))
    return false;

  if (PeekToken() == TokenType::QuestionMark) {
    // ConditionalExpression ::=
    //     NullCoalescingExpression ||
    //     NullCoalescingExpression '?' Expression ':' Expression
    auto const cond_part = ConsumeExpression();
    auto const op_question = ConsumeToken();
    if (!ParseExpression())
      return false;
    auto const then_part = ConsumeExpression();
    if (!AdvanceIf(TokenType::Colon))
      return Error(ErrorCode::ExpressionConditionalColon);
    if (ParseExpression())
      return false;
    auto const else_part = ConsumeExpression();
    ProduceExpression(factory()->NewConditional(op_question, cond_part,
                                                then_part, else_part));
    return true;
  }

  if (PeekTokenCategory() == ExpressionCategory::Assignment) {
    // Assignment ::= UnaryExpression AssignmentOperator Expression
    // AssignmentOperator ::= '=' | '+=' | '*=' | '/=' | '/=' ...
    // Note: Assignment is right-associative |a = b = c| == |a = (b = c)|.
    auto const op_assign = ConsumeToken();
    auto const lhs = ConsumeExpression();
    // TODO(eval1749) Check |lhs| is unary expression.
    if (!ParseExpression())
      return false;
    auto const rhs = ConsumeExpression();
    ProduceExpression(factory()->NewAssignment(op_assign, lhs, rhs));
    return true;
  }

  return !!expression_;
}

// Null-Coalescing-Expression ::=
//    Conditional-Or-Expression ||
//    Conditional-Or-Expression '??' Null-Coalescing-Expression
bool Parser::ParseExpressionSub(ExpressionCategory category) {
  if (category == ExpressionCategory::Primary)
    return ParsePrimaryExpression();

  if (category == ExpressionCategory::Unary) {
    if (PeekTokenCategory() == ExpressionCategory::Unary) {
      auto const op_token = ConsumeToken();
      if (!ParsePrimaryExpression())
        return false;
      ProduceUnaryOperation(op_token, ConsumeExpression());
      return true;
    }

    if (PeekToken() == TokenType::Add) {
      auto const op_token = ConsumeTokenAs(TokenType::UnaryAdd);
      if (!ParsePrimaryExpression())
        return false;
      ProduceUnaryOperation(op_token, ConsumeExpression());
      return true;
    }

    if (PeekToken() == TokenType::Sub) {
      auto const op_token = ConsumeTokenAs(TokenType::UnarySub);
      if (!ParsePrimaryExpression())
        return false;
      ProduceUnaryOperation(op_token, ConsumeExpression());
      return true;
    }

    return ParsePrimaryExpression();
  }

  // Handle Left-associative binary operators
  if (!ParseExpressionSub(RaisePrecedence(category)))
    return false;
  while (PeekTokenCategory() == category) {
    auto const op_token = ConsumeToken();
    auto const left = ConsumeExpression();
    if (!ParseExpressionSub(category))
      return false;
    ProduceBinaryOperation(op_token, left, ConsumeExpression());
  }
  return true;
}

// PrimaryExpression ::=
//    ArrayCreationExpression |
//    PrimaryNoArrayCreationExpression |
//
// PrimaryNoArrayCreationExpression :=
//    Literal
//    AtomicString
//    ParenthesizedExpression
//    member-access
//    invocation-expression
//    element-access
//    this-access
//    super-access
//    PostIncrement-expression
//    PostDecrement-expression
//    object-creation-expression
//    function-creation-expression
//    typeof-expression
//    default-value-expression
//    anonymous-method-expression
bool Parser::ParsePrimaryExpression() {
  if (PeekToken()->is_literal()) {
    ProduceExpression(factory()->NewLiteral(ConsumeToken()));
    return ParsePrimaryExpressionPost();
  }

  if (PeekToken()->is_name()) {
    ProduceExpression(factory()->NewNameReference(ConsumeToken()));
    return ParsePrimaryExpressionPost();
  }

  if (AdvanceIf(TokenType::LeftParenthesis)) {
    // ParenthesizedExpression:
    // '(' Expression ')'
    ParseExpression();
    if (!ParsePrimaryExpressionPost())
      return false;
    if (AdvanceIf(TokenType::RightParenthesis))
      return true;
    Error(ErrorCode::ExpressionPrimaryRightParenthesis);
    return false;
  }

  return false;
}

bool Parser::ParsePrimaryExpressionPost() {
  for (;;) {
    if (AdvanceIf(TokenType::Dot)) {
      // MemberAccess ::=
      //    PrimaryExpression '.' Identifier TypeArgumentList? |
      //    PredefinedType '.' Identifier TypeArgumentList? |
      //    QualifiedAliasMember '.' Identifier TypeArgumentList?
      auto const container = ConsumeExpression();
      if (!PeekToken()->is_name()) {
        Error(ErrorCode::SyntaxMemberAccessName, ConsumeToken());
        return false;
      }
      auto const member_name = factory()->NewNameReference(ConsumeToken());
      ProduceMemberAccess({ container, member_name });
      if (!AdvanceIf(TokenType::LeftAngleBracket))
        continue;
      auto const generic_type = ConsumeExpression();
      std::vector<ast::Expression*> type_args;
      do {
        if (!ParseType())
          return false;
        type_args.push_back(ConsumeType());
      } while (AdvanceIf(TokenType::Comma));
      DCHECK(!type_args.empty());
      if (!AdvanceIf(TokenType::RightAngleBracket))
        Error(ErrorCode::SyntaxMemberAccessTypeArgument);
      ProduceExpression(factory()->NewConstructedType(generic_type, type_args));
      continue;
    }

    if (AdvanceIf(TokenType::LeftParenthesis)) {
      // InvokeExpression ::= PrimerExpresion '(' ArgumentList? ')'
      // ArgumentList ::= Expression (',' Expression)*
      auto const callee = ConsumeExpression();
      std::vector<ast::Expression*> arguments;
      while (!AdvanceIf(TokenType::RightParenthesis)) {
        if (!ParseExpression())
          break;
        arguments.push_back(ConsumeExpression());
      }
      ProduceExpression(factory()->NewCall(callee, arguments));
      continue;
    }

    if (PeekToken() == TokenType::Increment) {
      // PostIncrementExpression ::=
      //    PrimeryExpression '++'
      auto const op_token = ConsumeTokenAs(TokenType::PostIncrement);
      ProduceUnaryOperation(op_token, ConsumeExpression());
      continue;
    }

    if (PeekToken() == TokenType::Decrement) {
      // PostDecrementExpression ::=
      //    PrimeryExpression '--'
      auto const op_token = ConsumeTokenAs(TokenType::PostDecrement);
      ProduceUnaryOperation(op_token, ConsumeExpression());
      continue;
    }

    return true;
  }
}

Parser::ExpressionCategory Parser::PeekTokenCategory() {
  PeekToken();
  if (!PeekToken()->is_operator())
    return ExpressionCategory::None;
  return static_cast<ExpressionCategory>(PeekToken()->precedence());
}

ast::Expression* Parser::ProduceExpression(ast::Expression* expression) {
  DCHECK(!expression_);
  return expression_ = expression;
}

ast::Expression* Parser::ProduceBinaryOperation(Token* op_token,
                                                ast::Expression* left,
                                                ast::Expression* right) {
  return ProduceExpression(
      factory()->NewBinaryOperation(op_token, left, right));
}

ast::Expression* Parser::ProduceUnaryOperation(Token* op_token,
                                               ast::Expression* expression) {
  return ProduceExpression(factory()->NewUnaryOperation(op_token, expression));
}

}  // namespace compiler
}  // namespace elang
