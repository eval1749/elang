// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_NODES_H_
#define ELANG_COMPILER_IR_NODES_H_

#include <vector>

#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_unordered_set.h"
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
                                                     \
 private:                                            \
  /* |Factory| class if friend of concrete |Node| */ \
  /* class, for accessing constructor. */            \
  friend class Factory;                              \
  /* Visitor pattern */                              \
  void Accept(Visitor* visitor) final;

#define FOR_EACH_IR_STORAGE_CLASS(V) \
  V(Heap)                            \
  V(Register)                        \
  V(Stack)                           \
  V(Void)

enum class StorageClass {
#define V(Name) Name,
  FOR_EACH_IR_STORAGE_CLASS(V)
#undef V
};

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

//////////////////////////////////////////////////////////////////////
//
// Type
//
class Type : public Node {
  DECLARE_ABSTRACT_IR_NODE_CLASS(Type, Node);

 public:
  virtual bool IsSubtypeOf(const Type* other) const = 0;

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

  // base classes of this class
  const ZoneUnorderedSet<Class*>& base_classes() const { return base_classes_; }

  // Direct base classes of this class
  const ZoneVector<Class*>& direct_base_classes() const {
    return direct_base_classes_;
  }

  // Returns true if |this| is 'class'.
  bool is_class() const;

 private:
  Class(Zone* zone,
        ast::Class* ast_type,
        const std::vector<Class*>& direct_base_classes);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  ast::Class* const ast_class_;
  ZoneUnorderedSet<Class*> base_classes_;
  const ZoneVector<Class*> direct_base_classes_;

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
  ir::Class* base_type() const { return base_type_; }

 private:
  Enum(Zone* zone,
       ast::Enum* ast_enum,
       ir::Class* base_type,
       const std::vector<int64_t>& values);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  ast::Enum* const ast_enum_;
  ir::Class* const base_type_;
  const ZoneVector<int64_t> values_;

  DISALLOW_COPY_AND_ASSIGN(Enum);
};

//////////////////////////////////////////////////////////////////////
//
// Literal
//
class Literal final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Literal, Node);

 public:
  Token* data() const { return data_; }
  Type* type() const { return type_; }

 private:
  Literal(Type* type, Token* token);

  Token* const data_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Literal);
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
class Parameter final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Parameter, Node);

 public:
  bool operator==(const Parameter& other) const;
  bool operator!=(const Parameter& other) const;

  Value* default_value() const { return default_value_; }
  bool is_rest() const;
  ParameterKind kind() const;
  Token* name() const;
  int position() const;
  Type* type() const { return type_; }

  bool IsIdentical(const Parameter& other) const;

 private:
  Parameter(ast::Parameter* ast_parameter, Type* type, Value* default_value);

  ast::Parameter* const ast_parameter_;
  Value* const default_value_;
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
  struct Arity {
    // TODO(eval1749) We should move |Signature::Arity::kMaximum| to
    // public place.
    static const int kMaximum = 100;
    int maximum;
    int minimum;
  };

  bool operator==(const Signature& other) const;
  bool operator!=(const Signature& other) const;

  int maximum_arity() const { return arity_.minimum; }
  int minimum_arity() const { return arity_.maximum; }
  const ZoneVector<Parameter*>& parameters() const { return parameters_; }
  Type* return_type() { return return_type_; }

  bool IsIdenticalParameters(const Signature* other) const;

 private:
  Signature(Zone* zone,
            Type* return_type,
            const std::vector<Parameter*>& parameters);

  // Type
  bool IsSubtypeOf(const Type* other) const final;

  const Arity arity_;
  const ZoneVector<Parameter*> parameters_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(Signature);
};

//////////////////////////////////////////////////////////////////////
//
// Variable
//
class Variable final : public Node {
  DECLARE_CONCRETE_IR_NODE_CLASS(Variable, Node);

 public:
  ast::NamedNode* ast_node() const { return ast_node_; }
  StorageClass storage() const { return storage_; }
  Type* type() const { return type_; }

 private:
  Variable(Type* type, StorageClass storage, ast::NamedNode* ast_node);

  ast::NamedNode* const ast_node_;
  StorageClass const storage_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Variable);
};

}  // namespace ir
}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_IR_NODES_H_
