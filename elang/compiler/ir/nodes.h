// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_IR_NODES_H_
#define ELANG_COMPILER_IR_NODES_H_

#include <vector>

#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_vector.h"
#include "elang/compiler/ast/nodes_forward.h"
#include "elang/compiler/ir/nodes_forward.h"

namespace elang {
namespace compiler {
namespace ir {

#define DECLARE_ANALYZED_DATA_CLASS(self, super) \
  DECLARE_CASTABLE_CLASS(self, super);           \
                                                 \
 protected:                                      \
  ~self() = default;

#define DECLARE_ANALYZED_DATA_CONCRETE_CLASS(self, super) \
  DECLARE_ANALYZED_DATA_CLASS(self, super);               \
  friend class Factory;

//////////////////////////////////////////////////////////////////////
//
// Node
//
class Node : public Castable, public ZoneAllocated {
  DECLARE_ANALYZED_DATA_CLASS(Node, Castable);

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
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Type, Node);

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
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Class, Type);

 public:
  ast::Class* ast_class() const { return ast_class_; }
  const ZoneVector<Class*>& base_classes() const { return base_classes_; }
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
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Enum, Type);

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
// Parameter
//
class Parameter final : public Node {
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Parameter, Node);

 public:
  Value* default_value() const { return default_value_; }
  Token* name() const { return name_; }
  Type* type() const { return type_; }

 private:
  Parameter(Token* name, Type* type, Value* default_value);

  Value* const default_value_;
  Token* const name_;
  Type* const type_;

  DISALLOW_COPY_AND_ASSIGN(Parameter);
};

//////////////////////////////////////////////////////////////////////
//
// Signature
//
class Signature final : public Type {
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Signature, Type);

 public:
  Token* name() const { return name_; }
  const ZoneVector<Parameter*>& parameters() const { return parameters_; }
  Type* return_type() { return return_type_; }

 private:
  friend class CompilationSession;

  Signature(Zone* zone,
            Token* name,
            Type* return_type,
            const std::vector<Parameter*>& parameters);

  Token* const name_;
  ZoneVector<Parameter*> parameters_;
  Type* const return_type_;

  DISALLOW_COPY_AND_ASSIGN(Signature);
};

//////////////////////////////////////////////////////////////////////
//
// Value
//
class Value final : public Node {
  DECLARE_ANALYZED_DATA_CONCRETE_CLASS(Value, Node);

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
