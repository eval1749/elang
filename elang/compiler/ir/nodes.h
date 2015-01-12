// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_NODES_H_
#define ELANG_COMPILER_IR_NODES_H_

#include <ostream>
#include <vector>

#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/ir/nodes_forward.h"

namespace elang {
namespace compiler {
namespace ir {

#define DECLARE_IR_NODE_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);     \
                                           \
 protected:                                \
  ~self() = default;

#define DECLARE_ABSTRACT_IR_NODE_CLASS(self, super) \
  DECLARE_IR_NODE_CLASS(self, super);

#define DECLARE_CONCRETE_IR_NODE_CLASS(self, super)  \
  DECLARE_IR_NODE_CLASS(self, super);                \
 private:                                            \
  /* |Factory| class if friend of concrete |Node| */ \
  /* class, for accessing constructor. */            \
  friend class Factory;                              \
  /* Visitor pattern */                              \
  void Accept(Visitor* visitor) final;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public Visitable<Visitor>, public ZoneAllocated {
  DECLARE_ABSTRACT_IR_NODE_CLASS(Node, Castable);

 protected:
  Node();

 private:
  DISALLOW_COPY_AND_ASSIGN(Node);
};

// Print for formatting and debugging.
std::ostream& operator<<(std::ostream& ostream, const Node& node);

//////////////////////////////////////////////////////////////////////
//
// Type
//
class Type : public Node {
  DECLARE_ABSTRACT_IR_NODE_CLASS(Type, Node);

 protected:
  Type();

 private:
  DISALLOW_COPY_AND_ASSIGN(Type);
};

//////////////////////////////////////////////////////////////////////
//
// Class
//
class Class final : public Type {
  DECLARE_CONCRETE_IR_NODE_CLASS(Class, Type);

 public:
  // Associated AST class object.
  ast::Class* ast_class() const { return ast_class_; }

  // Direct base classes of this class
  const ZoneVector<Class*>& base_classes() const { return base_classes_; }

  // Returns true if |this| is 'class'.
  bool is_class() const;

 private:
  Class(Zone* zone,
        ast::Class* ast_type,
        const std::vector<Class*>& base_classes);

  ast::Class* const ast_class_;
  const ZoneVector<Class*> base_classes_;

  DISALLOW_COPY_AND_ASSIGN(Class);
};

//////////////////////////////////////////////////////////////////////
//
// Enum
//
class Enum final : public Type {
  DECLARE_CONCRETE_IR_NODE_CLASS(Enum, Type);

 public:
  ast::Enum* ast_enum() const { return ast_enum_; }

 private:
  Enum(Zone* zone, ast::Enum* ast_enum, const std::vector<int64_t>& values);

  ast::Enum* const ast_enum_;
  const ZoneVector<int64_t> values_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// Method
//
class Method final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Method, Node);

 public:
  ast::Method* ast_method() const { return ast_method_; }
  const ZoneVector<Parameter*>& parameters() const;
  Type* return_type() const;
  Signature* signature() const { return signature_; }

 private:
  Method(ast::Method* ast_method, Signature* signature);

  ast::Method* const ast_method_;
  Signature* const signature_;

  DISALLOW_COPY_AND_ASSIGN(Method);
};

//////////////////////////////////////////////////////////////////////
//
// Parameter
//
enum class ParameterKind {
  Required,
  Optional,
  Array,
};

class Parameter final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Parameter, Node);

 public:
  bool operator==(const Parameter& other) const;
  bool operator!=(const Parameter& other) const;

  Value* default_value() const { return default_value_; }
  ParameterKind kind() const { return kind_; }
  Token* name() const { return name_; }
  Type* type() const { return type_; }

  bool IsIdentical(const Parameter& other) const;

 private:
  Parameter(ParameterKind kind, Token* name, Type* type, Value* default_value);

  Value* const default_value_;
  ParameterKind const kind_;
  Token* const name_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Parameter);
};

//////////////////////////////////////////////////////////////////////
//
// Signature
//
class Signature final : public Type {
  DECLARE_CONCRETE_IR_NODE_CLASS(Signature, Type);

 public:
  bool operator==(const Signature& other) const;
  bool operator!=(const Signature& other) const;

  const ZoneVector<Parameter*>& parameters() const { return parameters_; }
  Type* return_type() { return return_type_; }

  bool IsIdenticalParameters(const Signature* other) const;

 private:
  Signature(Zone* zone,
            Type* return_type,
            const std::vector<Parameter*>& parameters);

  ZoneVector<Parameter*> parameters_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(Signature);
};

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Value, Node);

 public:
  Token* value() const { return value_; }

 private:
  explicit Value(Token* value);

  Token* const value_;

  DISALLOW_COPY_AND_ASSIGN(Value);
};

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_NODES_H_
