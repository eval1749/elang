// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <deque>
#include <memory>
#include <unordered_set>
#include <vector>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analyze/class_analyzer.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/testing/namespace_builder.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {

std::vector<ir::Class*> ComputeBaseClassList(
    const ZoneVector<ir::Class*>& direct_base_classes) {
  std::vector<ir::Class*> base_classes(direct_base_classes.size());
  base_classes.resize(0);
  std::unordered_set<ir::Class*> seen;
  std::deque<ir::Class*> pending_classes(direct_base_classes.begin(),
                                         direct_base_classes.end());
  while (!pending_classes.empty()) {
    auto const current = pending_classes.front();
    pending_classes.pop_front();
    if (seen.count(current))
      continue;
    base_classes.push_back(current);
    seen.insert(current);
    for (auto base_class : current->direct_base_classes()) {
      if (seen.count(base_class))
        continue;
      pending_classes.push_back(base_class);
    }
  }
  DCHECK_GE(base_classes.size(), direct_base_classes.size());
  return base_classes;
}

class SystemNamespaceBuilder final : public NamespaceBuilder {
 public:
  explicit SystemNamespaceBuilder(NameResolver* name_resolver);
  ~SystemNamespaceBuilder() = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(SystemNamespaceBuilder);
};

SystemNamespaceBuilder::SystemNamespaceBuilder(NameResolver* name_resolver)
    : NamespaceBuilder(name_resolver) {
  NewClass("Object", "");
  NewClass("ValueType", "Object");
  NewClass("Enum", "ValueType");

  NewClass("Bool", "ValueType");
  NewClass("Char", "ValueType");
  NewClass("Float32", "ValueType");
  NewClass("Float64", "ValueType");
  NewClass("Int16", "ValueType");
  NewClass("Int32", "ValueType");
  NewClass("Int64", "ValueType");
  NewClass("Int8", "ValueType");
  NewClass("UInt16", "ValueType");
  NewClass("UInt32", "ValueType");
  NewClass("UInt64", "ValueType");
  NewClass("UInt8", "ValueType");
  NewClass("Void", "ValueType");

  NewClass("String", "Object");
}

std::unique_ptr<NameResolver> NewNameResolver(CompilationSession* session) {
  auto resolver = std::make_unique<NameResolver>(session);
  SystemNamespaceBuilder builder(resolver.get());
  return resolver;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AnalyzerTest::ClassOrStrig
//
AnalyzerTest::ClassOrString::ClassOrString(const char* format,
                                           base::StringPiece name)
    : ir_class(nullptr),
      message(base::StringPrintf(format, name.as_string().c_str())) {
}

//////////////////////////////////////////////////////////////////////
//
// AnalyzerTest
//
AnalyzerTest::AnalyzerTest() : name_resolver_(NewNameResolver(session())) {
}

AnalyzerTest::~AnalyzerTest() {
}

std::string AnalyzerTest::AnalyzeClass() {
  if (!Parse())
    return GetErrors();
  ClassAnalyzer resolver(name_resolver());
  return resolver.Run() ? "" : GetErrors();
}

std::string AnalyzerTest::AnalyzeNamespace() {
  if (!Parse())
    return GetErrors();
  NamespaceAnalyzer resolver(name_resolver());
  return resolver.Run() ? "" : GetErrors();
}

std::string AnalyzerTest::GetBaseClasses(base::StringPiece name) {
  auto const thing = GetClass(name);
  if (!thing.ir_class)
    return thing.message;
  return MakeClassListString(
      ComputeBaseClassList(thing.ir_class->direct_base_classes()));
}

AnalyzerTest::ClassOrString AnalyzerTest::GetClass(base::StringPiece name) {
  auto const member = FindMember(name);
  if (!member)
    return ClassOrString("No such class %s", name);
  auto const ast_class = member->as<ast::Class>();
  if (!ast_class)
    return ClassOrString("%s isn't class", name);
  auto const resolved = name_resolver_->Resolve(ast_class);
  if (!resolved)
    return ClassOrString("%s isn't resolved", name);
  auto const ir_class = resolved->as<ir::Class>();
  if (!ir_class)
    return ClassOrString("%s isn't resolved to class", name);
  return ClassOrString(ir_class);
}

std::string AnalyzerTest::GetDirectBaseClasses(base::StringPiece name) {
  auto const thing = GetClass(name);
  if (!thing.ir_class)
    return thing.message;
  return MakeClassListString(thing.ir_class->direct_base_classes());
}

std::string AnalyzerTest::GetMethodGroup(base::StringPiece name) {
  auto const ast_node = FindMember(name);
  if (!ast_node)
    return base::StringPrintf("%s isn't found", name.as_string().c_str());
  auto const ast_method_group = ast_node->as<ast::MethodGroup>();
  if (!ast_method_group) {
    return base::StringPrintf("%s isn't method group",
                              name.as_string().c_str());
  }
  std::stringstream ostream;
  for (auto const ast_method : ast_method_group->methods()) {
    auto const ir_method = name_resolver()->Resolve(ast_method);
    if (!ir_method) {
      ostream << "Not resolved " << ast_method->token() << std::endl;
      continue;
    }
    ostream << *ir_method << std::endl;
  }
  return ostream.str();
}

std::string AnalyzerTest::MakeClassListString(
    const std::vector<ir::Class*>& ir_classes) {
  std::stringstream ostream;
  auto separator = "";
  for (auto const ir_base_class : ir_classes) {
    ostream << separator;
    ostream << ir_base_class->ast_class()->NewQualifiedName();
    separator = " ";
  }
  return ostream.str();
}

std::string AnalyzerTest::MakeClassListString(
    const ZoneVector<ir::Class*>& classes) {
  return MakeClassListString(
      std::vector<ir::Class*>(classes.begin(), classes.end()));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
