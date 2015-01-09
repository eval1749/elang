// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/compiler/testing/compiler_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/qualified_name.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/string_source_code.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/compiler/testing/formatter.h"
#include "elang/compiler/token.h"
#include "elang/compiler/token_type.h"

namespace elang {
namespace compiler {
namespace testing {

namespace {
std::string GetQualifiedName(ast::NamespaceMember* member) {
  std::vector<Token*> names;
  names.push_back(member->name());
  for (auto ns = member->outer(); ns && ns->outer(); ns = ns->outer())
    names.push_back(ns->name());
  std::reverse(names.begin(), names.end());
  std::stringstream stream;
  const char* separator = "";
  for (auto const name : names) {
    stream << separator << name;
    separator = ".";
  }
  return stream.str();
}

//////////////////////////////////////////////////////////////////////
//
// SystemNamespaceBuilder
//
class SystemNamespaceBuilder final {
 public:
  explicit SystemNamespaceBuilder(CompilationSession* session);
  ~SystemNamespaceBuilder() = default;

  void Build();

 private:
  ast::Class* GetOrNewClass(ast::MemberContainer* container,
                            base::StringPiece16 name);
  ast::Namespace* GetOrNewNamespace(ast::Namespace* container,
                                    base::StringPiece16 name);
  ast::Namespace* NewSystemNamespace();
  Token* NewToken(TokenData token_data);
  Token* NewToken(TokenType token_type);
  CompilationSession* const session_;

  DISALLOW_COPY_AND_ASSIGN(SystemNamespaceBuilder);
};

SystemNamespaceBuilder::SystemNamespaceBuilder(CompilationSession* session)
    : session_(session) {
}

void SystemNamespaceBuilder::Build() {
  auto const ns_global = session_->global_namespace();
  ns_global->AddNamespaceBody(
      session_->ast_factory()->NewNamespaceBody(nullptr, ns_global));
  auto const ns_system = GetOrNewNamespace(ns_global, L"System");
  ns_system->AddNamespaceBody(session_->ast_factory()->NewNamespaceBody(
      ns_global->bodies().front(), ns_system));
  GetOrNewClass(ns_system, L"Object");
  GetOrNewClass(ns_system, L"Value");
  GetOrNewClass(ns_system, L"Bool");
  GetOrNewClass(ns_system, L"Char");
  GetOrNewClass(ns_system, L"Float32");
  GetOrNewClass(ns_system, L"Float64");
  GetOrNewClass(ns_system, L"Int16");
  GetOrNewClass(ns_system, L"Int32");
  GetOrNewClass(ns_system, L"Int64");
  GetOrNewClass(ns_system, L"Int8");
  GetOrNewClass(ns_system, L"String");
  GetOrNewClass(ns_system, L"UInt16");
  GetOrNewClass(ns_system, L"UInt32");
  GetOrNewClass(ns_system, L"UInt64");
  GetOrNewClass(ns_system, L"UInt8");
  GetOrNewClass(ns_system, L"Void");
}

ast::Class* SystemNamespaceBuilder::GetOrNewClass(
    ast::MemberContainer* container,
    base::StringPiece16 name) {
  auto const name_string = session_->NewAtomicString(name);
  if (auto const member = container->FindMember(name_string)) {
    if (auto const clazz = member->as<ast::Class>())
      return clazz;
    DCHECK(!member);
  }
  auto const new_class = session_->ast_factory()->NewClass(
      container->bodies().front(), Modifiers(), NewToken(TokenType::Class),
      NewToken(TokenData(name_string)));
  container->AddMember(new_class);
  return new_class;
}

ast::Namespace* SystemNamespaceBuilder::GetOrNewNamespace(
    ast::Namespace* container,
    base::StringPiece16 name) {
  auto const name_string = session_->NewAtomicString(name);
  if (auto const member = container->FindMember(name_string)) {
    if (auto const clazz = member->as<ast::Namespace>())
      return clazz;
    DCHECK(!member);
  }
  auto const new_namespace = session_->ast_factory()->NewNamespace(
      container->bodies().front(), NewToken(TokenType::Namespace),
      NewToken(TokenData(name_string)));
  container->AddMember(new_namespace);
  return new_namespace;
}

Token* SystemNamespaceBuilder::NewToken(TokenData token_data) {
  return session_->NewToken(SourceCodeRange(), token_data);
}

Token* SystemNamespaceBuilder::NewToken(TokenType token_type) {
  return NewToken(TokenData(token_type));
}

ast::Namespace* SystemNamespaceBuilder::NewSystemNamespace() {
  return GetOrNewNamespace(session_->global_namespace(), L"System");
}

NameResolver* NewNameResolver(CompilationSession* session) {
  SystemNamespaceBuilder builder(session);
  builder.Build();
  return new NameResolver(session);
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompilerTest
//
CompilerTest::CompilerTest()
    : session_(new CompilationSession()),
      name_resolver_(NewNameResolver(session_.get())) {
}

CompilerTest::~CompilerTest() {
}

SourceCode* CompilerTest::source_code() const {
  return source_code_.get();
}

std::string CompilerTest::AnalyzeNamespace() {
  if (!Parse())
    return GetErrors();
  NamespaceAnalyzer resolver(session_.get(), name_resolver_.get());
  return resolver.Run() ? "" : GetErrors();
}

ast::Class* CompilerTest::FindClass(base::StringPiece name) {
  auto const member = FindMember(name);
  return member ? member->as<ast::Class>() : nullptr;
}

ast::NamespaceMember* CompilerTest::FindMember(base::StringPiece name) {
  auto enclosing =
      static_cast<ast::MemberContainer*>(session_->global_namespace());
  auto found = static_cast<ast::NamespaceMember*>(nullptr);
  for (size_t pos = 0u; pos < name.length(); ++pos) {
    auto dot_pos = name.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = name.length();
    auto const simple_name = session_->NewAtomicString(
        base::UTF8ToUTF16(name.substr(pos, dot_pos - pos)));
    found = enclosing->FindMember(simple_name);
    if (!found)
      return nullptr;
    enclosing = found->as<ast::MemberContainer>();
    if (!enclosing)
      return nullptr;
    pos = dot_pos;
  }
  return found;
}

std::string CompilerTest::Format(base::StringPiece source_code) {
  Prepare(source_code);
  return Format();
}

std::string CompilerTest::Format() {
  if (!Parse())
    return GetErrors();
  Formatter formatter;
  return formatter.Run(session_->global_namespace());
}

std::string CompilerTest::GetBaseClasses(base::StringPiece name) {
  auto const member = FindMember(name);
  if (!member)
    return base::StringPrintf("No such class %s", name.as_string().c_str());
  auto const clazz = member->as<ast::Class>();
  if (!clazz)
    return base::StringPrintf("%s isn't class", name.as_string().c_str());
  std::stringstream stream;
  const char* separator = "";
  for (auto const base_class_name : clazz->base_class_names()) {
    stream << separator;
    if (auto const base_class = name_resolver_->FindReference(base_class_name))
      stream << GetQualifiedName(base_class);
    else
      stream << "Not resolved " << base_class_name->token();
    separator = ", ";
  }
  return stream.str();
}

std::string CompilerTest::GetErrors() {
  static const char* const error_messages[] = {
#define E(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_COMPILER_ERROR_CODE(E, E)
#undef E
  };

  std::stringstream stream;
  for (auto const error : session_->errors()) {
    stream << error_messages[static_cast<int>(error->error_code())] << "("
           << error->location().start().offset() << ")";
    for (auto token : error->tokens())
      stream << " " << token;
    stream << std::endl;
  }
  return stream.str();
}

bool CompilerTest::Parse() {
  auto const compilation_unit =
      session_->NewCompilationUnit(source_code_.get());
  Parser parser(session_.get(), compilation_unit);
  return parser.Run();
}

void CompilerTest::Prepare(base::StringPiece16 source_text) {
  source_code_.reset(new StringSourceCode(L"testing", source_text));
}

void CompilerTest::Prepare(base::StringPiece source_text) {
  source_code_.reset(
      new StringSourceCode(L"testing", base::UTF8ToUTF16(source_text)));
}

}  // namespace testing
}  // namespace compiler
}  // namespace elang
