// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ostream>

#include "elang/compiler/syntax/parser.h"

#include "base/logging.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/local_variable.h"
#include "elang/compiler/ast/node_factory.h"
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
  if (!ParseExpressionSub(ExpressionCategory::NullCoalescing)) {
    DCHECK(!expression_);
    return false;
  }

  if (PeekToken() == TokenType::QuestionMark) {
    // ConditionalExpression ::=
    //     NullCoalescingExpression ||
    //     NullCoalescingExpression '?' Expression ':' Expression
    auto const cond_part = ConsumeExpression();
    auto const op_question = ConsumeToken();
    if (!ParseExpression()) {
      Error(ErrorCode::SyntaxExpressionConditionalThen);
      DCHECK(!expression_);
      return false;
    }
    auto const then_part = ConsumeExpression();
    if (!AdvanceIf(TokenType::Colon))
      return Error(ErrorCode::SyntaxExpressionConditionalColon);
    if (!ParseExpression()) {
      Error(ErrorCode::SyntaxExpressionConditionalElse);
      DCHECK(!expression_);
      return false;
    }
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
    if (!ParseExpression()) {
      DCHECK(!expression_);
      return false;
    }
    auto const rhs = ConsumeExpression();
    ProduceExpression(factory()->NewAssignment(op_assign, lhs, rhs));
    return true;
  }

  return !!expression_;
}

bool Parser::ParseExpressionSub(ExpressionCategory category) {
  if (category == ExpressionCategory::Primary)
    return ParsePrimaryExpression();

  if (category == ExpressionCategory::Unary)
    return ParseUnaryExpression();

  // Handle Left-associative binary operators
  if (!ParseExpressionSub(RaisePrecedence(category))) {
    DCHECK(!expression_);
    return false;
  }
  while (PeekTokenCategory() == category) {
    auto const op_token = ConsumeToken();
    auto const left = ConsumeExpression();
    if (!ParseExpressionSub(category))
      return true;
    ProduceBinaryOperation(op_token, left, ConsumeExpression());
  }
  DCHECK(expression_);
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
    ParsePrimaryExpressionPost();
    return true;
  }

  if (PeekToken()->is_name()) {
    // NameReference
    auto const name = ConsumeToken();
    if (auto const local_member = FindLocalMember(name)) {
      // Local name reference
      if (auto const var = local_member->as<ast::LocalVariable>()) {
        ProduceVariableReference(name, var);
      } else {
        Error(ErrorCode::SyntaxExpressionLabel, name);
        ProduceExpression(factory()->NewInvalidExpression(name));
      }
      ParsePrimaryExpressionPost();
      return true;
    }

    // Non-local name reference
    ProduceExpression(factory()->NewNameReference(name));
    ParsePrimaryExpressionName();
    ParsePrimaryExpressionPost();
    return true;
  }

  if (PeekToken()->is_type_name()) {
    // Type name: 'bool', 'char', 'int', 'int16', and so on.
    ProduceType(factory()->NewNameReference(ConsumeToken()));
    if (AdvanceIf(TokenType::LeftAngleBracket)) {
      // TODO(eval1749) Skip until right angle bracket
      Error(ErrorCode::SyntaxExpressionLeftAngleBracket);
    }
    ParseTypePost();
    return true;
  }

  if (auto const parenthesis = ConsumeTokenIf(TokenType::LeftParenthesis)) {
    // ParenthesizedExpression:
    // '(' Expression ')'
    if (!ParseExpression())
      ProduceExpression(factory()->NewInvalidExpression(parenthesis));
    ParsePrimaryExpressionPost();
    if (!AdvanceIf(TokenType::RightParenthesis))
      Error(ErrorCode::SyntaxExpressionRightParenthesis);
    return true;
  }

  DCHECK(!expression_);
  return false;
}

void Parser::ParsePrimaryExpressionName() {
  if (!AdvanceIf(TokenType::LeftAngleBracket))
    return;
  auto const generic_type = ConsumeExpression();
  std::vector<ast::Expression*> type_args;
  do {
    if (!ParseType()) {
      // TODO(eval1749) Skip to right angle bracket.
      break;
    }
    type_args.push_back(ConsumeType());
  } while (AdvanceIf(TokenType::Comma));
  if (type_args.empty()) {
    Error(ErrorCode::SyntaxMemberAccessTypeArgument);
    return;
  }
  if (!AdvanceIf(TokenType::RightAngleBracket))
    Error(ErrorCode::SyntaxMemberAccessRightAngleBracket);
  ProduceExpression(factory()->NewConstructedType(generic_type, type_args));
}

void Parser::ParsePrimaryExpressionPost() {
  for (;;) {
    if (AdvanceIf(TokenType::Dot)) {
      // MemberAccess ::=
      //    PrimaryExpression '.' Identifier TypeArgumentList? |
      //    PredefinedType '.' Identifier TypeArgumentList? |
      //    QualifiedAliasMember '.' Identifier TypeArgumentList?
      if (!PeekToken()->is_name()) {
        Error(ErrorCode::SyntaxMemberAccessName, ConsumeToken());
        return;
      }
      auto const container = ConsumeExpression();
      auto const member_name = factory()->NewNameReference(ConsumeToken());
      ProduceMemberAccess({container, member_name});
      ParsePrimaryExpressionName();
      continue;
    }

    if (AdvanceIf(TokenType::LeftParenthesis)) {
      // InvokeExpression ::= PrimerExpresion '(' ArgumentList? ')'
      // ArgumentList ::= Expression (',' Expression)*
      auto const callee = ConsumeExpression();
      std::vector<ast::Expression*> arguments;
      if (PeekToken() != TokenType::RightParenthesis) {
        do {
          if (!ParseExpression())
            break;
          arguments.push_back(ConsumeExpression());
        } while (AdvanceIf(TokenType::Comma));
      }
      if (!AdvanceIf(TokenType::RightParenthesis))
        Error(ErrorCode::SyntaxExpressionRightParenthesis);
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

    if (auto const bracket = ConsumeTokenIf(TokenType::LeftSquareBracket)) {
      if (PeekToken() == TokenType::RightSquareBracket ||
          PeekToken() == TokenType::Comma) {
        ParseArrayType(bracket);
        continue;
      }
      // TODO(eval1749) NYI Array reference
    }

    if (auto const bracket = ConsumeTokenIf(TokenType::LeftAngleBracket)) {
      // TODO(eval1749) Skip until right angle bracket
      Error(ErrorCode::SyntaxExpressionLeftAngleBracket, bracket);
    }

    // Here, we have token not part of primary expression.
    return;
  }
}

// UnaryExpression ::=
//  PrimaryExpression |
//  '++' UnaryExpression |
//  '--' UnaryExpression |
//  '+' UnaryExpression |
//  '-' UnaryExpression |
//  '!' UnaryExpression |
//  '~' UnaryExpression |
//  dynamic_cast<Type>(Expression) |
//  static_cast<Type>(Expression)
bool Parser::ParseUnaryExpression() {
  auto const op_token = TryConsumeUnaryOperator();
  if (!op_token)
    return ParsePrimaryExpression();
  if (!ParseUnaryExpression())
    return false;
  ProduceUnaryOperation(op_token, ConsumeExpression());
  return true;
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

Token* Parser::TryConsumeUnaryOperator() {
  if (PeekTokenCategory() == ExpressionCategory::Unary)
    return ConsumeToken();
  if (PeekToken() == TokenType::Add)
    return ConsumeTokenAs(TokenType::UnaryAdd);
  if (PeekToken() == TokenType::Sub)
    return ConsumeTokenAs(TokenType::UnarySub);
  return nullptr;
}

}  // namespace compiler
}  // namespace elang
