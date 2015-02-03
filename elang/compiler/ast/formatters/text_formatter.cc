// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

#include "elang/compiler/ast/formatters/text_formatter.h"

#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/enum.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/types.h"
#include "elang/compiler/ast/visitor.h"

namespace elang {
namespace compiler {
namespace ast {

namespace {

std::string GetQualifiedName(ast::NamedNode* node) {
  return base::UTF16ToUTF8(node->NewQualifiedName());
}

//////////////////////////////////////////////////////////////////////
//
// Formatter
//
class Formatter final : public ast::Visitor {
 public:
  explicit Formatter(std::ostream* ostream);
  ~Formatter() = default;

  void Format(const ast::Node* node);

 private:
  // ast::Visitor
  void VisitCall(ast::Call* node);
  void VisitClass(ast::Class* node);
  void VisitLiteral(ast::Literal* node);
  void VisitImport(ast::Import* node);
  void VisitMemberAccess(ast::MemberAccess* node);
  void VisitMethod(ast::Method* node);
  void VisitMethodGroup(ast::MethodGroup* node);
  void VisitNameReference(ast::NameReference* node);
  void VisitNamespace(ast::Namespace* node);
  void VisitTypeMemberAccess(ast::TypeMemberAccess* node);
  void VisitTypeNameReference(ast::TypeNameReference* node);

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::Formatter(std::ostream* ostream) : ostream_(*ostream) {
}

void Formatter::Format(const ast::Node* node) {
  const_cast<ast::Node*>(node)->Accept(this);
}

// Visitor
void Formatter::VisitCall(ast::Call* node) {
  ostream_ << *node->callee() << "(";
  auto separator = "";
  for (auto const argument : node->arguments()) {
    ostream_ << separator << *argument;
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitClass(ast::Class* node) {
  ostream_ << "class " << GetQualifiedName(node);
}

void Formatter::VisitImport(ast::Import* node) {
  ostream_ << "using " << *node->reference();
}

void Formatter::VisitLiteral(ast::Literal* node) {
  ostream_ << *node->token();
}

void Formatter::VisitMemberAccess(ast::MemberAccess* node) {
  auto separator = "";
  for (auto const component : node->components()) {
    ostream_ << separator << *component;
    separator = ".";
  }
}

void Formatter::VisitMethod(ast::Method* node) {
  ostream_ << "method " << node->modifiers() << " " << *node->return_type()
           << " " << GetQualifiedName(node) << "(";
  auto separator = "";
  for (auto const parameter : node->parameters()) {
    ostream_ << separator << *parameter->type();
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitMethodGroup(ast::MethodGroup* node) {
  ostream_ << "method group " << GetQualifiedName(node);
}
void Formatter::VisitNameReference(ast::NameReference* node) {
  ostream_ << *node->name();
}

void Formatter::VisitNamespace(ast::Namespace* node) {
  ostream_ << "namespace " << GetQualifiedName(node);
}

void Formatter::VisitTypeMemberAccess(ast::TypeMemberAccess* node) {
  VisitMemberAccess(node->reference());
}

void Formatter::VisitTypeNameReference(ast::TypeNameReference* node) {
  VisitNameReference(node->reference());
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  Formatter formatter(&ostream);
  formatter.Format(&node);
  return ostream;
}

//////////////////////////////////////////////////////////////////////
//
// TextFormatter
//
TextFormatter::TextFormatter(std::ostream* ostream) : ostream_(*ostream) {
}

TextFormatter::~TextFormatter() {
}

void TextFormatter::Format(const Node* node) {
  Formatter formatter(&ostream_);
  formatter.Format(node);
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
