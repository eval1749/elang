// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <string>
#include <sstream>
#include <vector>

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
  void DoDefaultVisit(ast::Node* node) final;
  void VisitAlias(ast::Alias* node) final;
  void VisitArrayAccess(ast::ArrayAccess* node) final;
  void VisitArrayType(ast::ArrayType* node) final;
  void VisitCall(ast::Call* node) final;
  void VisitClass(ast::Class* node) final;
  void VisitEnum(ast::Enum* node) final;
  void VisitEnumMember(ast::EnumMember* node) final;
  void VisitLiteral(ast::Literal* node) final;
  void VisitImport(ast::Import* node) final;
  void VisitMemberAccess(ast::MemberAccess* node) final;
  void VisitMethod(ast::Method* node) final;
  void VisitNameReference(ast::NameReference* node) final;
  void VisitNamespaceBody(ast::NamespaceBody* node) final;
  void VisitParameter(ast::Parameter* node) final;
  void VisitParameterReference(ast::ParameterReference* node) final;
  void VisitTypeMemberAccess(ast::TypeMemberAccess* node) final;
  void VisitTypeNameReference(ast::TypeNameReference* node) final;
  void VisitTypeVariable(ast::TypeVariable* node) final;

  std::ostream& ostream_;

  DISALLOW_COPY_AND_ASSIGN(Formatter);
};

Formatter::Formatter(std::ostream* ostream) : ostream_(*ostream) {
}

void Formatter::Format(const ast::Node* node) {
  Traverse(node);
}

// Visitor

void Formatter::DoDefaultVisit(ast::Node* node) {
  ostream_ << node->class_name() << "@" << static_cast<void*>(&node);
}

void Formatter::VisitAlias(ast::Alias* node) {
  ostream_ << "using " << node->name() << " = " << node->reference();
}

void Formatter::VisitArrayAccess(ast::ArrayAccess* node) {
  ostream_ << node->array() << "[";
  auto separator = "";
  for (auto index : node->indexes()) {
    ostream_ << index;
    separator = ", ";
  }
  ostream_ << "]";
}

// Element type of array type is omitting left most rank, e.g.
//  element_type_of(T[A]) = T
//  element_type_of(T[A][B}) = T[B]
//  element_type_of(T[A][B}[C]) = T[B][C]
void Formatter::VisitArrayType(ast::ArrayType* node) {
  std::vector<ArrayType*> array_types;
  for (Type* runner = node; runner->is<ArrayType>();
       runner = runner->as<ArrayType>()->element_type()) {
    array_types.push_back(runner->as<ArrayType>());
  }
  ostream_ << *array_types.back()->element_type();
  for (auto array_type : array_types) {
    ostream_ << "[";
    auto separator = "";
    for (auto dimension : array_type->dimensions()) {
      ostream_ << separator;
      if (dimension >= 0)
        ostream_ << dimension;
      separator = ",";
    }
    ostream_ << "]";
  }
}

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
  ostream_ << "class " << GetQualifiedName(node) << " {...}";
}

void Formatter::VisitEnum(ast::Enum* node) {
  ostream_ << "enum " << node->name();
  if (!node->enum_base())
    return;
  ostream_ << " : " << *node->enum_base();
}

void Formatter::VisitEnumMember(ast::EnumMember* node) {
  ostream_ << "enum " << GetQualifiedName(node->parent()->as<ast::Enum>())
           << " {" << node->name() << "}";
}

void Formatter::VisitImport(ast::Import* node) {
  ostream_ << "using " << *node->reference();
}

void Formatter::VisitLiteral(ast::Literal* node) {
  ostream_ << *node->token();
}

void Formatter::VisitMemberAccess(ast::MemberAccess* node) {
  Traverse(node->container());
  ostream_ << "." << node->member();
}

void Formatter::VisitMethod(ast::Method* node) {
  ostream_ << "method";
  if (node->modifiers().value())
    ostream_ << " " << node->modifiers();
  ostream_ << " " << *node->return_type() << " " << GetQualifiedName(node)
           << "(";
  auto separator = "";
  for (auto const parameter : node->parameters()) {
    ostream_ << separator << *parameter->type();
    separator = ", ";
  }
  ostream_ << ")";
}

void Formatter::VisitNameReference(ast::NameReference* node) {
  ostream_ << *node->name();
}

void Formatter::VisitNamespaceBody(ast::NamespaceBody* node) {
  if (!node->parent()) {
    ostream_ << "global_namespace";
    return;
  }
  ostream_ << "namespace " << GetQualifiedName(node);
}

void Formatter::VisitParameter(ast::Parameter* node) {
  ostream_ << *node->name() << " {...}";
}

void Formatter::VisitParameterReference(ast::ParameterReference* node) {
  ostream_ << *node->parameter();
}

void Formatter::VisitTypeMemberAccess(ast::TypeMemberAccess* node) {
  VisitMemberAccess(node->reference());
}

void Formatter::VisitTypeNameReference(ast::TypeNameReference* node) {
  VisitNameReference(node->reference());
}

void Formatter::VisitTypeVariable(ast::TypeVariable* node) {
  ostream_ << "var";
}

}  // namespace

std::ostream& operator<<(std::ostream& ostream, const Node& node) {
  Formatter formatter(&ostream);
  formatter.Format(&node);
  return ostream;
}

std::ostream& operator<<(std::ostream& ostream, const Node* node) {
  if (!node)
    return ostream << "nil";
  return ostream << *node;
}

}  // namespace ast
}  // namespace compiler
}  // namespace elang
