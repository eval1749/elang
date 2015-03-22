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
#include "base/strings/stringprintf.h"
#include "elang/base/zone_allocated.h"
#include "elang/cg/generator.h"
#include "elang/compiler/analysis/name_resolver.h"
#include "elang/compiler/ast/class.h"
#include "elang/compiler/ast/factory.h"
#include "elang/compiler/ast/method.h"
#include "elang/compiler/ast/namespace.h"
#include "elang/compiler/compilation_session.h"
#include "elang/compiler/compilation_unit.h"
#include "elang/compiler/semantics/factory.h"
#include "elang/compiler/semantics/nodes.h"
#include "elang/compiler/public/compiler_error_code.h"
#include "elang/compiler/public/compiler_error_data.h"
#include "elang/compiler/semantics.h"
#include "elang/compiler/source_code.h"
#include "elang/compiler/source_code_position.h"
#include "elang/compiler/syntax/parser.h"
#include "elang/compiler/token_type.h"
#include "elang/hir/error_data.h"
#include "elang/hir/factory.h"
#include "elang/hir/factory_config.h"
#include "elang/hir/formatters/text_formatter.h"
#include "elang/hir/types.h"
#include "elang/hir/type_factory.h"
#include "elang/hir/values.h"
#include "elang/lir/error_data.h"
#include "elang/lir/factory.h"
#include "elang/lir/formatters/text_formatter.h"
#include "elang/shell/namespace_builder.h"
#include "elang/shell/node_query.h"
#include "elang/shell/source_file_stream.h"
#include "elang/vm/factory.h"
#include "elang/vm/machine_code_function.h"
#include "elang/vm/machine_code_builder_impl.h"
#include "elang/vm/objects.h"
#include "elang/vm/object_factory.h"

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
  builder.NewClass("IntPtr", "ValueType");
  builder.NewClass("UInt16", "ValueType");
  builder.NewClass("UInt32", "ValueType");
  builder.NewClass("UInt64", "ValueType");
  builder.NewClass("UInt8", "ValueType");
  builder.NewClass("UIntPtr", "ValueType");
  builder.NewClass("Void", "ValueType");

  builder.NewClass("String", "Object");

  // public class Console {
  //   public static void WriteLine(String string);
  //   public static void WriteLine(String string, Object object);
  // }
  auto const console_class_body = builder.NewClass("Console", "Object");
  auto const console_class = console_class_body->owner();

  auto const write_line = builder.ast_factory()->NewMethodGroup(
      console_class, builder.NewName("WriteLine"));

  auto const write_line_string = builder.ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      builder.NewTypeReference(TokenType::Void), write_line->name(), {});
  write_line_string->SetParameters({
      builder.NewParameter(write_line_string, 0, "System.String", "string"),
  });

  auto const write_line_string_object = builder.ast_factory()->NewMethod(
      console_class_body, write_line,
      Modifiers(Modifier::Extern, Modifier::Public, Modifier::Static),
      builder.NewTypeReference(TokenType::Void), write_line->name(), {});
  write_line_string_object->SetParameters({
      builder.NewParameter(write_line_string_object, 0, "System.String",
                           "string"),
      builder.NewParameter(write_line_string_object, 1, "System.Object",
                           "object"),
  });

  write_line->AddMethod(write_line_string);
  console_class_body->AddMember(write_line_string);
  write_line->AddMethod(write_line_string_object);
  console_class_body->AddMember(write_line_string_object);
  console_class->AddNamedMember(write_line);
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
          ->as<sm::Type>();
  auto const string_type =
      semantics->ValueOf(session->QueryAstNode(L"System.String"))
          ->as<sm::Type>();
  auto const string_array_type =
      name_resolver->factory()->NewArrayType(string_type, {-1});
  auto const void_type =
      semantics->ValueOf(session->QueryAstNode(L"System.Void"))->as<sm::Type>();

  MethodQuery query1(name_main, void_type, {ParameterQuery(void_type)});
  MethodQuery query2(name_main, void_type, {ParameterQuery(string_array_type)});
  MethodQuery query3(name_main, int32_type, {ParameterQuery(void_type)});
  MethodQuery query4(name_main, int32_type,
                     {ParameterQuery(string_array_type)});
  OrQuery query({&query1, &query2, &query3, &query4});
  return QueryAllNodes(session, &query);
}

lir::Function* Generate(lir::Factory* factory, hir::Function* hir_function) {
  ::elang::cg::Generator generator(factory, hir_function);
  return generator.Generate();
}

vm::MachineCodeFunction* GenerateMachineCode(vm::Factory* vm_factory,
                                             lir::Factory* lir_factory,
                                             lir::Function* lir_function) {
  vm::MachineCodeBuilderImpl mc_builder(vm_factory);
  lir_factory->GenerateMachineCode(&mc_builder, lir_function);
  return mc_builder.NewMachineCodeFunction();
}

bool ReportHirError(const hir::Factory* factory) {
  if (factory->errors().empty())
    return false;
  for (auto const error : factory->errors())
    std::cerr << *error << std::endl;
  return true;
}

bool ReportLirError(const lir::Factory* factory) {
  if (factory->errors().empty())
    return false;
  for (auto const error : factory->errors())
    std::cerr << *error << std::endl;
  return true;
}

}  // namespace

Compiler::Compiler(const std::vector<base::string16>& args)
    : args_(args),
      is_dump_hir_(false),
      is_dump_lir_(false),
      session_(new CompilationSession()) {
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

  if (ReportHirError(factory.get()))
    return 1;

  // Find entry point.
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
  if (is_dump_hir_) {
    hir::TextFormatter formatter(&std::cout);
    formatter.FormatFunction(main_function);
  }

  // Translate HIR to LIR
  std::unique_ptr<lir::Factory> lir_factory(new lir::Factory());
  auto const lir_function = Generate(lir_factory.get(), main_function);

  if (ReportLirError(lir_factory.get()))
    return 1;

  factory.release();

  if (is_dump_lir_) {
    lir::TextFormatter formatter(lir_factory->literals(), &std::cout);
    formatter.FormatFunction(lir_function);
  }

  // Translate LIR to Machine code
  std::unique_ptr<vm::Factory> vm_factory(new vm::Factory());
  auto const mc_function =
      GenerateMachineCode(vm_factory.get(), lir_factory.get(), lir_function);

  // Dump machine code
  auto const code_start = mc_function->code_start_for_testing();
  auto const code_end = code_start + mc_function->code_size_for_testing();
  for (auto code = code_start; code < code_end; ++code) {
    if ((code - code_start) % 16 == 0)
      std::cout << std::endl;
    std::cout << " " << base::StringPrintf("%02X", *code);
  }
  std::cout << std::endl;

  // Execute
  if (main_function->parameters_type()->is<hir::VoidType>()) {
    if (main_function->return_type()->is<hir::VoidType>()) {
      mc_function->Invoke();
      return 0;
    }
    return mc_function->Call<int>();
  }
  DCHECK_GE(args_.size(), 1);
  auto const objects = vm_factory->object_factory();
  auto const args = objects->NewVector<vm::impl::String*>(
      objects->string_class(), args_.size() - 1);
  for (auto index = 1; index < args_.size(); ++index)
    (*args)[index - 1] = objects->NewString(base::StringPiece16(args_[index]));

  if (main_function->return_type()->is<hir::VoidType>()) {
    mc_function->Invoke(args);
    return 0;
  }

  return mc_function->Call<int, vm::impl::Vector<vm::impl::String*>*>(args);
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
