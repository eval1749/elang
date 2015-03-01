// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <iterator>
#include <vector>

#include "elang/shell/compiler.h"

#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "elang/base/zone_allocated.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/ir/factory.h"
#include "elang/compiler/ir/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"
#include "elang/shell/namespace_builder.h"
#include "elang/shell/node_query.h"
#include "elang/shell/source_file_stream.h"

namespace elang {
namespace compiler {
namespace shell {

namespace {

//////////////////////////////////////////////////////////////////////
//
// ReadableErrorData
//
struct ReadableErrorData {
  const ErrorData* error_data;

  explicit ReadableErrorData(const ErrorData& data) : error_data(&data) {}
};

std::ostream& operator<<(std::ostream& ostream,
                         const ReadableErrorData& readable) {
  static const char* const mnemonics[] = {
#define V(category, subcategory, name) #category "." #subcategory "." #name,
      FOR_EACH_COMPILER_ERROR_CODE(V, V)
#undef V
  };
  auto const it = std::begin(mnemonics) +
                  static_cast<size_t>(readable.error_data->error_code());
  ostream << (it < std::end(mnemonics) ? *it : "InvalidErrorCode");
  for (auto const token : readable.error_data->tokens())
    ostream << " " << token;
  return ostream;
}

//////////////////////////////////////////////////////////////////////
//
// FileSourceCode
//
class FileSourceCode final : public ::elang::compiler::SourceCode,
                             public ZoneAllocated {
 public:
  explicit FileSourceCode(const base::FilePath& file_path);
  ~FileSourceCode() = default;

  const base::FilePath& file_path() const { return stream().file_path(); }
  const SourceFileStream& stream() const { return *stream_; }

 private:
  // elang::compiler::SourceCode
  ::elang::compiler::CharacterStream* GetStream() final;

  base::FilePath file_path_;
  std::unique_ptr<SourceFileStream> stream_;

  DISALLOW_COPY_AND_ASSIGN(FileSourceCode);
};

FileSourceCode::FileSourceCode(const base::FilePath& file_path)
    : SourceCode(file_path.value()), stream_(new SourceFileStream(file_path)) {
}

::elang::compiler::CharacterStream* FileSourceCode::GetStream() {
  return stream_.get();
}

std::unique_ptr<hir::FactoryConfig> NewFactoryConfig(
    CompilationSession* session) {
  auto config = std::make_unique<hir::FactoryConfig>();
  config->atomic_string_factory = session->atomic_string_factory();
  config->string_type_name = session->NewAtomicString(L"System.String");
  return config;
}

// TODO(eval1749) We should load "System" namespace from file instead of
// building here.
void PopulateNamespace(NameResolver* name_resolver) {
  NamespaceBuilder builder(name_resolver);

  builder.NewClass("Object", "");
  builder.NewClass("ValueType", "Object");
  builder.NewClass("Enum", "ValueType");

  builder.NewClass("Bool", "ValueType");
  builder.NewClass("Char", "ValueType");
  builder.NewClass("Float32", "ValueType");
  builder.NewClass("Float64", "ValueType");
  builder.NewClass("Int16", "ValueType");
  builder.NewClass("Int32", "ValueType");
  builder.NewClass("Int64", "ValueType");
  builder.NewClass("Int8", "ValueType");
  builder.NewClass("UInt16", "ValueType");
  builder.NewClass("UInt32", "ValueType");
  builder.NewClass("UInt64", "ValueType");
  builder.NewClass("UInt8", "ValueType");
  builder.NewClass("Void", "ValueType");

  builder.NewClass("String", "Object");
}

// Collect methods having following signature:
//  - void Main()
//  - void Main(String[])
//  - int Main()
//  - int Main(String[])
// Note: In HIR, objects are passed as pointers rather than object.
std::vector<ast::Node*> CollectMainMethods(CompilationSession* session,
                                           NameResolver* name_resolver) {
  auto const semantics = session->semantics();
  auto const name_main = session->NewAtomicString(L"Main");
  auto const int32_type =
      semantics->ValueOf(session->QueryAstNode(L"System.Int32"))
          ->as<ir::Type>();
  auto const string_type =
      semantics->ValueOf(session->QueryAstNode(L"System.String"))
          ->as<ir::Type>();
  auto const string_array_type =
      name_resolver->factory()->NewArrayType(string_type, {-1});
  auto const void_type =
      semantics->ValueOf(session->QueryAstNode(L"System.Void"))->as<ir::Type>();

  MethodQuery query1(name_main, void_type, {ParameterQuery(void_type)});
  MethodQuery query2(name_main, void_type, {ParameterQuery(string_array_type)});
  MethodQuery query3(name_main, int32_type, {ParameterQuery(void_type)});
  MethodQuery query4(name_main, int32_type,
                     {ParameterQuery(string_array_type)});
  OrQuery query({&query1, &query2, &query3, &query4});
  return QueryAllNodes(session, &query);
}

}  // namespace

Compiler::Compiler() : session_(new CompilationSession()) {
}

Compiler::~Compiler() {
}

void Compiler::AddSourceFile(const base::FilePath& file_path) {
  auto const source_code = new (session()->zone()) FileSourceCode(file_path);
  if (!source_code->stream().file().IsValid()) {
    std::cerr << "Unable to open file "
              << source_code->stream().file_path().value() << "("
              << base::File::ErrorToString(
                     source_code->stream().file().error_details()) << ")"
              << std::endl;
    return;
  }
  auto const compilation_unit = session()->NewCompilationUnit(source_code);
  compiler::Parser parser(session(), compilation_unit);
  parser.Run();
}

int Compiler::CompileAndGo() {
  if (!session()->errors().empty())
    return 1;

  std::unique_ptr<hir::FactoryConfig> factory_config(
      NewFactoryConfig(session()));
  std::unique_ptr<hir::Factory> factory(new hir::Factory(*factory_config));

  NameResolver name_resolver(session());
  PopulateNamespace(&name_resolver);

  if (!session()->Compile(&name_resolver, factory.get()))
    return 1;

  auto const main_methods = CollectMainMethods(session(), &name_resolver);
  if (main_methods.empty()) {
    std::cerr << "No Main method." << std::endl;
    return 1;
  }
  if (main_methods.size() > 1u) {
    std::cerr << "More than one main methods:" << std::endl;
    for (auto const method : main_methods)
      std::cerr << "  " << *method << std::endl;
    return 1;
  }
  auto const main_method = main_methods.front()->as<ast::Method>();
  auto const main_function = session()->FunctionOf(main_method);
  if (!main_function) {
    std::cerr << "No function for main method." << *main_method;
    return 1;
  }
  hir::TextFormatter formatter(&std::cout);
  formatter.FormatFunction(main_function);
  return 0;
}

void Compiler::ReportErrors() {
  if (session()->errors().empty())
    return;

  for (auto const error : session()->errors()) {
    auto const& location = error->location();
    std::cerr << location.source_code()->name() << "("
              << location.start().line() + 1
              << "): " << ReadableErrorData(*error) << std::endl;
  }
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
