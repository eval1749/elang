// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <deque>
#include <memory>
#include <unordered_set>
#include <vector>

#include "elang/compiler/testing/compiler_test.h"

#include "base/logging.h"
#include "base/macros.h"
#include "base/strings/stringprintf.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/base/simple_directed_graph.h"
#include "elang/compiler/analyze/namespace_analyzer.h"
#include "elang/compiler/analyze/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/expressions.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/ast/node_factory.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/modifiers_builder.h"
#include "elang/compiler/predefined_names.h"
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
    for (auto base_class : current->base_classes()) {
      if (seen.count(base_class))
        continue;
      pending_classes.push_back(base_class);
    }
  }
  DCHECK_GE(base_classes.size(), direct_base_classes.size());
  return base_classes;
}

std::string ConvertErrorListToString(const std::vector<ErrorData*> errors) {
  static const char* const mnemonic[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_COMPILER_ERROR_CODE(V, V)
#undef V
  };

  std::stringstream stream;
  for (auto const error : errors) {
    auto const index = static_cast<int>(error->error_code());
    stream << mnemonic[index] << "(" << error->location().start().offset()
           << ")";
    for (auto token : error->tokens())
      stream << " " << token;
    stream << std::endl;
  }
  return stream.str();
}

std::string GetQualifiedName(ast::NamedNode* member) {
  std::vector<Token*> names;
  names.push_back(member->name());
  for (auto runner = member->parent(); runner; runner = runner->parent()) {
    if (auto const ns_body = runner->as<ast::NamespaceBody>())
      runner = ns_body->owner();
    if (!runner->parent())
      break;
    names.push_back(runner->name());
  }
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
// IrClassBuilder
//
class IrClassBuilder final : public ast::Visitor {
 public:
  explicit IrClassBuilder(NameResolver* resolver);
  ~IrClassBuilder() = default;

  void Run();

 private:
  CompilationSession* session() const { return resolver_->session(); }

  void DidResolved(ast::NamedNode* ast_node, ir::Node* node);
  void Postpone(ast::NamedNode* user, ast::NamedNode* used);
  void ProcessClass(ast::Class* ast_class);
  ast::NamedNode* Resolve(ast::Expression* reference,
                          ast::ContainerNode* container);

  // ast::Visitor
  void VisitClass(ast::Class* node);
  void VisitNamespaceBody(ast::NamespaceBody* node);

  bool collecting_;
  SimpleDirectedGraph<ast::NamedNode*> node_graph_;
  NameResolver* const resolver_;
  std::unordered_set<ast::NamedNode*> unresolved_nodes_;
  std::vector<ast::Class*> pending_classes_;

  DISALLOW_COPY_AND_ASSIGN(IrClassBuilder);
};

IrClassBuilder::IrClassBuilder(NameResolver* resolver)
    : collecting_(true), resolver_(resolver) {
}

void IrClassBuilder::DidResolved(ast::NamedNode* ast_node, ir::Node* node) {
  resolver_->DidResolve(ast_node, node);
  unresolved_nodes_.erase(ast_node);
  for (auto const user : node_graph_.GetInEdges(ast_node)) {
    node_graph_.RemoveEdge(user, ast_node);
    if (node_graph_.HasOutEdge(user))
      continue;
    user->Accept(this);
  }
}

void IrClassBuilder::Postpone(ast::NamedNode* user, ast::NamedNode* used) {
  node_graph_.AddEdge(user, used);
  unresolved_nodes_.insert(user);
}

void IrClassBuilder::ProcessClass(ast::Class* ast_class) {
  if (resolver_->Resolve(ast_class))
    return;
  auto const ast_enclosing_class = ast_class->parent()->as<ast::Class>();
  if (ast_enclosing_class && !resolver_->Resolve(ast_enclosing_class))
    Postpone(ast_class, ast_enclosing_class);

  std::vector<ir::Class*> base_classes;
  for (auto const base_class_name : ast_class->base_class_names()) {
    auto const ast_base_class = Resolve(base_class_name, ast_class);
    DCHECK(ast_base_class);
    auto const base_class = resolver_->Resolve(ast_base_class);
    if (base_class) {
      base_classes.push_back(base_class->as<ir::Class>());
      continue;
    }
    Postpone(ast_class, ast_base_class);
  }
  if (node_graph_.HasOutEdge(ast_class))
    return;
  DidResolved(ast_class,
              resolver_->factory()->NewClass(ast_class, base_classes));
}

ast::NamedNode* IrClassBuilder::Resolve(ast::Expression* reference,
                                        ast::ContainerNode* container) {
  auto const name_ref = reference->as<ast::NameReference>();
  DCHECK(name_ref);
  auto const name = name_ref->name();
  for (auto runner = container; runner; runner = runner->parent()) {
    if (auto const member = runner->FindMember(name))
      return member;
  }
  return nullptr;
}

void IrClassBuilder::Run() {
  VisitNamespaceBody(session()->root_node());
  collecting_ = false;
  for (auto ast_class : pending_classes_)
    ProcessClass(ast_class);
  DCHECK(unresolved_nodes_.empty());
}

// ast::Visitor
void IrClassBuilder::VisitClass(ast::Class* ast_class) {
  if (collecting_) {
    pending_classes_.push_back(ast_class);
    return;
  }
  ProcessClass(ast_class);
}

void IrClassBuilder::VisitNamespaceBody(ast::NamespaceBody* ns_body) {
  ns_body->AcceptForMembers(this);
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
  ast::Class* FindClass(PredefinedName name);
  void FixValueClass(PredefinedName name);
  void InstallPredefinedName(PredefinedName name);
  ast::NameReference* NewNameReference(PredefinedName name);
  Token* NewToken(TokenData token_data);
  Token* NewToken(TokenType token_type);

  CompilationSession* const session_;
  ast::NamespaceBody* system_namespace_body_;

  DISALLOW_COPY_AND_ASSIGN(SystemNamespaceBuilder);
};

SystemNamespaceBuilder::SystemNamespaceBuilder(CompilationSession* session)
    : session_(session),
      system_namespace_body_(session->ast_factory()->NewNamespaceBody(
          session->root_node(),
          session->system_namespace())) {
  system_namespace_body_->loaded_ = true;
  session_->root_node()->AddMember(system_namespace_body_);
}

void SystemNamespaceBuilder::Build() {
#define V(Name) InstallPredefinedName(PredefinedName::Name);
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
  // The base class of |String| and |Value| classes are |Object|.
  FindClass(PredefinedName::String)
      ->AddBaseClassName(NewNameReference(PredefinedName::Object));
  FindClass(PredefinedName::Value)
      ->AddBaseClassName(NewNameReference(PredefinedName::Object));
#define V(Name) FixValueClass(PredefinedName::Name);
  FOR_EACH_PREDEFINED_NAME(V)
#undef V
}

ast::Class* SystemNamespaceBuilder::FindClass(PredefinedName name_id) {
  auto const name = session_->name_for(name_id);
  return session_->system_namespace()->FindMember(name)->as<ast::Class>();
}

void SystemNamespaceBuilder::FixValueClass(PredefinedName name) {
  auto const ast_class = FindClass(name);
  if (!ast_class->base_class_names().empty())
    return;
  auto const object_name = session_->name_for(PredefinedName::Object);
  if (ast_class->name()->atomic_string() == object_name)
    return;
  ast_class->AddBaseClassName(NewNameReference(PredefinedName::Value));
}

void SystemNamespaceBuilder::InstallPredefinedName(PredefinedName type) {
  ModifiersBuilder modifiers_builder;
  modifiers_builder.SetPublic();
  auto const container = session_->system_namespace();
  auto const modifiers = modifiers_builder.Get();
  auto const name = session_->name_for(type);
  auto const class_string = session_->NewAtomicString(L"class");
  auto const new_class = session_->ast_factory()->NewClass(
      container, modifiers, NewToken(TokenData(TokenType::Class, class_string)),
      NewToken(TokenData(name)));
  container->AddNamedMember(new_class);
  system_namespace_body_->AddMember(new_class);
}

ast::NameReference* SystemNamespaceBuilder::NewNameReference(
    PredefinedName name) {
  return session_->ast_factory()->NewNameReference(
      NewToken(TokenData(session_->name_for(name))));
}

Token* SystemNamespaceBuilder::NewToken(TokenData token_data) {
  return session_->NewToken(SourceCodeRange(), token_data);
}

Token* SystemNamespaceBuilder::NewToken(TokenType token_type) {
  return NewToken(TokenData(token_type));
}

std::unique_ptr<NameResolver> NewNameResolver(CompilationSession* session) {
  SystemNamespaceBuilder builder(session);
  builder.Build();
  auto resolver = std::make_unique<NameResolver>(session);
  IrClassBuilder installer(resolver.get());
  installer.Run();
  return resolver;
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// CompilerTest::ClassOrStrig
//
CompilerTest::ClassOrString::ClassOrString(const char* format,
                                           base::StringPiece name)
    : ir_class(nullptr),
      message(base::StringPrintf(format, name.as_string().c_str())) {
}

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
  NamespaceAnalyzer resolver(name_resolver_.get());
  return resolver.Run() ? "" : GetErrors();
}

ast::Class* CompilerTest::FindClass(base::StringPiece name) {
  auto const member = FindMember(name);
  return member ? member->as<ast::Class>() : nullptr;
}

ast::NamedNode* CompilerTest::FindMember(base::StringPiece name) {
  auto enclosing =
      static_cast<ast::ContainerNode*>(session_->global_namespace());
  auto found = static_cast<ast::NamedNode*>(nullptr);
  for (size_t pos = 0u; pos < name.length(); ++pos) {
    auto dot_pos = name.find('.', pos);
    if (dot_pos == base::StringPiece::npos)
      dot_pos = name.length();
    auto const simple_name = session_->NewAtomicString(
        base::UTF8ToUTF16(name.substr(pos, dot_pos - pos)));
    found = enclosing->FindMember(simple_name);
    if (!found)
      return nullptr;
    enclosing = found->as<ast::ContainerNode>();
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
  return formatter.Run(session_->root_node());
}

std::string CompilerTest::GetBaseClasses(base::StringPiece name) {
  auto const thing = GetClass(name);
  if (!thing.ir_class)
    return thing.message;
  return MakeClassListString(
      ComputeBaseClassList(thing.ir_class->base_classes()));
}

CompilerTest::ClassOrString CompilerTest::GetClass(base::StringPiece name) {
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

std::string CompilerTest::GetDirectBaseClasses(base::StringPiece name) {
  auto const thing = GetClass(name);
  if (!thing.ir_class)
    return thing.message;
  return MakeClassListString(thing.ir_class->base_classes());
}

std::string CompilerTest::GetErrors() {
  return ConvertErrorListToString(session_->errors());
}

std::string CompilerTest::GetWarnings() {
  return ConvertErrorListToString(session_->warnings());
}

std::string CompilerTest::MakeClassListString(
    const std::vector<ir::Class*>& ir_classes) {
  std::stringstream stream;
  const char* separator = "";
  for (auto const ir_base_class : ir_classes) {
    stream << separator;
    stream << GetQualifiedName(ir_base_class->ast_class());
    separator = " ";
  }
  return stream.str();
}

std::string CompilerTest::MakeClassListString(
    const ZoneVector<ir::Class*>& classes) {
  return MakeClassListString(
      std::vector<ir::Class*>(classes.begin(), classes.end()));
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
