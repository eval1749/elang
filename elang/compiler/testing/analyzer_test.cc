// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <deque>
#include <memory>
#include <sstream>
#include <unordered_set>
#include <vector>

#include "elang/compiler/testing/analyzer_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analysis/analysis.h"
#include "elang/compiler/analysis/class_analyzer.h"
#include "elang/compiler/analysis/method_analyzer.h"
#include "elang/compiler/analysis/namespace_analyzer.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/namespace_builder.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {

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
  NewStruct("Enum", "ValueType");

  NewStruct("Bool", "ValueType");
  NewStruct("Char", "ValueType");
  NewStruct("Float32", "ValueType");
  NewStruct("Float64", "ValueType");
  NewStruct("Int16", "ValueType");
  NewStruct("Int32", "ValueType");
  NewStruct("Int64", "ValueType");
  NewStruct("Int8", "ValueType");
  NewStruct("IntPtr", "ValueType");
  NewStruct("UInt16", "ValueType");
  NewStruct("UInt32", "ValueType");
  NewStruct("UInt64", "ValueType");
  NewStruct("UInt8", "ValueType");
  NewStruct("UIntPtr", "ValueType");
  NewStruct("Void", "ValueType");

  NewClass("String", "Object");
}

std::unique_ptr<NameResolver> NewNameResolver(CompilationSession* session) {
  auto resolver = std::make_unique<NameResolver>(session);
  SystemNamespaceBuilder builder(resolver.get());
  return resolver;
}

template <typename Pass>
bool RunPass(NameResolver* name_resolver) {
  Pass(name_resolver).Run();
  return !name_resolver->session()->HasError();
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// AnalyzerTest
//
AnalyzerTest::AnalyzerTest() : name_resolver_(NewNameResolver(session())) {
}

AnalyzerTest::~AnalyzerTest() {
}

Analysis* AnalyzerTest::analysis() const {
  return session()->analysis();
}

std::string AnalyzerTest::Analyze() {
  if (!Parse())
    return GetErrors();
  if (!RunPass<NamespaceAnalyzer>(name_resolver()))
    return GetErrors();
  if (!RunPass<ClassAnalyzer>(name_resolver()))
    return GetErrors();
  if (!RunPass<MethodAnalyzer>(name_resolver()))
    return GetErrors();
  return "";
}

std::string AnalyzerTest::AnalyzeClass() {
  if (!Parse())
    return GetErrors();
  if (!RunPass<NamespaceAnalyzer>(name_resolver()))
    return GetErrors();
  if (!RunPass<ClassAnalyzer>(name_resolver()))
    return GetErrors();
  return "";
}

std::string AnalyzerTest::AnalyzeNamespace() {
  if (!Parse())
    return GetErrors();
  if (!RunPass<NamespaceAnalyzer>(name_resolver()))
    return GetErrors();
  return "";
}

std::string AnalyzerTest::MakeClassListString(
    const std::vector<sm::Class*>& ir_classes) {
  std::stringstream ostream;
  auto separator = "";
  for (auto const ir_base_class : ir_classes) {
    ostream << separator << ir_base_class;
    separator = " ";
  }
  return ostream.str();
}

std::string AnalyzerTest::MakeClassListString(
    const ZoneVector<sm::Class*>& classes) {
  return MakeClassListString(
      std::vector<sm::Class*>(classes.begin(), classes.end()));
}

sm::Semantic* AnalyzerTest::SemanticOf(base::StringPiece16 path) const {
  sm::Semantic* enclosing = session()->semantic_factory()->global_namespace();
  sm::Semantic* found = static_cast<sm::Semantic*>(nullptr);
  for (size_t pos = 0u; pos < path.length(); ++pos) {
    auto dot_pos = path.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = path.length();
    auto const name =
        session()->NewAtomicString(path.substr(pos, dot_pos - pos));
    found = enclosing->FindMember(name);
    if (!found)
      return nullptr;
    pos = dot_pos;
    if (pos == path.length())
      break;
    enclosing = found;
    if (!enclosing)
      return nullptr;
  }
  return found;
}

sm::Semantic* AnalyzerTest::SemanticOf(base::StringPiece path) const {
  return SemanticOf(base::UTF8ToUTF16(path));
}

std::string AnalyzerTest::ToString(sm::Semantic* semantic) {
  std::stringstream ostream;
  ostream << semantic;
  return ostream.str();
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
