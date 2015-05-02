// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <iterator>
#include <vector>

#include "elang/shell/compiler.h"

#include "base/command_line.h"
#include "base/files/file_path.h"
#include "base/files/file_util.h"
#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "base/strings/string_split.h"
#include "elang/api/pass.h"
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
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/types.h"
#include "elang/shell/namespace_builder.h"
#include "elang/shell/node_query.h"
#include "elang/shell/source_file_stream.h"
#include "elang/translator/translator.h"
#include "elang/vm/factory.h"
#include "elang/vm/machine_code_function.h"
#include "elang/vm/machine_code_builder_impl.h"
#include "elang/vm/objects.h"
#include "elang/vm/object_factory.h"

namespace elang {
namespace compiler {
namespace shell {

namespace {

namespace ir = optimizer;

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

std::unique_ptr<ir::FactoryConfig> NewIrFactoryConfig(
    CompilationSession* session) {
  auto config = std::make_unique<ir::FactoryConfig>();
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

  MethodQuery query1(name_main, void_type, {});
  MethodQuery query2(name_main, void_type, {ParameterQuery(string_array_type)});
  MethodQuery query3(name_main, int32_type, {});
  MethodQuery query4(name_main, int32_type,
                     {ParameterQuery(string_array_type)});
  OrQuery query({&query1, &query2, &query3, &query4});
  return QueryAllNodes(session, &query);
}

ast::Method* FindMainMethod(CompilationSession* session,
                            NameResolver* name_resolver) {
  auto const main_methods = CollectMainMethods(session, name_resolver);
  if (main_methods.empty()) {
    std::cerr << "No Main method." << std::endl;
    return nullptr;
  }
  if (main_methods.size() > 1u) {
    std::cerr << "More than one main methods:" << std::endl;
    for (auto const method : main_methods)
      std::cerr << "  " << *method << std::endl;
    return nullptr;
  }
  return main_methods.front()->as<ast::Method>();
}

lir::Function* TranslateToLir(lir::Factory* factory,
                              hir::Function* hir_function) {
  ::elang::cg::Generator generator(factory, hir_function);
  return generator.Generate();
}

lir::Function* TranslateToLir(lir::Factory* factory, ir::Schedule* schedule) {
  return ::elang::translator::Translator(factory, schedule).Run();
}

vm::MachineCodeFunction* GenerateMachineCode(vm::Factory* vm_factory,
                                             lir::Factory* lir_factory,
                                             lir::Function* lir_function) {
  vm::MachineCodeBuilderImpl mc_builder(vm_factory);
  lir_factory->GenerateMachineCode(&mc_builder, lir_function);
  return mc_builder.NewMachineCodeFunction();
}

bool ReportHirErrors(const hir::Factory* factory) {
  if (factory->errors().empty())
    return false;
  for (auto const error : factory->errors())
    std::cerr << *error << std::endl;
  return true;
}

bool ReportIrErrors(const ir::Factory* factory) {
  if (factory->errors().empty())
    return false;
  for (auto const error : factory->errors())
    std::cerr << *error << std::endl;
  return true;
}

bool ReportLirErrors(const lir::Factory* factory) {
  if (factory->errors().empty())
    return false;
  for (auto const error : factory->errors())
    std::cerr << *error << std::endl;
  return true;
}

std::vector<std::string> SwitchValuesOf(base::StringPiece switch_name) {
  auto const command_line = base::CommandLine::ForCurrentProcess();
  std::vector<std::string> values;
  base::SplitString(command_line->GetSwitchValueASCII(switch_name.as_string()),
                    ',', &values);
  return std::move(values);
}

const char kDumpHir[] = "dump_hir";
const char kDumpLir[] = "dump_lir";
const char kUseHir[] = "use_hir";

}  // namespace

Compiler::Compiler(const std::vector<base::string16>& args)
    : args_(args), session_(new CompilationSession()) {
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
  if (ReportCompileErrors())
    return 1;

  auto const command_line = base::CommandLine::ForCurrentProcess();

  NameResolver name_resolver(session());
  PopulateNamespace(&name_resolver);

  std::unique_ptr<lir::Factory> lir_factory(new lir::Factory());
  auto lir_function = static_cast<lir::Function*>(nullptr);
  auto has_parameter = false;
  auto has_return_value = false;

  // --dump=pass[,pass]*
  // --dump_after=pass[,pass]*
  // --dump_before=pass[,pass]*
  for (auto name : SwitchValuesOf("dump")) {
    dump_after_passes_.insert(name);
    dump_before_passes_.insert(name);
  }
  for (auto name : SwitchValuesOf("dump_after"))
    dump_after_passes_.insert(name);
  for (auto name : SwitchValuesOf("dump_before"))
    dump_before_passes_.insert(name);

  // --graph=pass[,pass]*
  // --graph_after=pass[,pass]*
  // --graph_before=pass[,pass]*
  for (auto name : SwitchValuesOf("graph")) {
    graph_after_passes_.insert(name);
    graph_before_passes_.insert(name);
  }
  for (auto name : SwitchValuesOf("graph_after"))
    graph_after_passes_.insert(name);
  for (auto name : SwitchValuesOf("graph_before"))
    graph_before_passes_.insert(name);

  if (!command_line->HasSwitch(kUseHir)) {
    // Compile to Optimizer-IR
    auto const factory_config = NewIrFactoryConfig(session());
    auto const factory = std::make_unique<ir::Factory>(this, *factory_config);
    session()->Compile(&name_resolver, factory.get());
    if (ReportCompileErrors())
      return 1;
    if (ReportIrErrors(factory.get()))
      return 1;
    auto const main_method = FindMainMethod(session(), &name_resolver);
    if (!main_method)
      return 1;
    auto const main_function = session()->IrFunctionOf(main_method);
    if (!main_function) {
      std::cerr << "No function for main method." << *main_method;
      return 1;
    }

    // Translate IR to LIR
    auto const schedule = factory->ComputeSchedule(main_function);
    lir_function = TranslateToLir(lir_factory.get(), schedule.get());
    has_parameter = !main_function->parameters_type()->is<ir::VoidType>();
    has_return_value = !main_function->return_type()->is<ir::VoidType>();

  } else {
    // Compile to HIR
    auto const factory_config = NewFactoryConfig(session());
    auto const factory = std::make_unique<hir::Factory>(*factory_config);

    session()->Compile(&name_resolver, factory.get());
    if (ReportCompileErrors())
      return 1;

    if (ReportHirErrors(factory.get()))
      return 1;

    // Find main method
    auto const main_method = FindMainMethod(session(), &name_resolver);
    if (!main_method)
      return 1;
    auto const main_function = session()->FunctionOf(main_method);
    if (!main_function) {
      std::cerr << "No function for main method." << *main_method;
      return 1;
    }
    if (command_line->HasSwitch(kDumpHir)) {
      hir::TextFormatter formatter(&std::cout);
      formatter.FormatFunction(main_function);
    }

    // Translate HIR to LIR
    lir_function = TranslateToLir(lir_factory.get(), main_function);
    has_parameter = !main_function->parameters_type()->is<hir::VoidType>();
    has_return_value = !main_function->return_type()->is<hir::VoidType>();
  }

  if (ReportLirErrors(lir_factory.get()))
    return 1;

  if (command_line->HasSwitch(kDumpLir)) {
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
  if (!has_parameter) {
    if (has_return_value)
      return mc_function->Call<int>();
    mc_function->Invoke();
    return 0;
  }

  DCHECK_GE(args_.size(), 1);
  auto const objects = vm_factory->object_factory();
  auto const args = objects->NewVector<vm::impl::String*>(
      objects->string_class(), args_.size() - 1);
  for (auto index = 1; index < args_.size(); ++index)
    (*args)[index - 1] = objects->NewString(base::StringPiece16(args_[index]));

  if (!has_return_value) {
    mc_function->Invoke(args);
    return 0;
  }

  return mc_function->Call<int, vm::impl::Vector<vm::impl::String*>*>(args);
}

bool Compiler::ReportCompileErrors() {
  if (session()->errors().empty())
    return false;

  for (auto const error : session()->errors()) {
    auto const& location = error->location();
    std::cerr << location.source_code()->name() << "("
              << location.start().line() + 1
              << "): " << ReadableErrorData(*error) << std::endl;
  }
  return true;
}

// api::PassObserver implementation
void Compiler::DidEndPass(api::Pass* pass) {
  auto const pass_name = pass->name();
  DVLOG(0) << "End " << pass_name << " " << pass->duration().InMillisecondsF()
           << "ms";
  if (dump_after_passes_.count(pass_name.as_string())) {
    api::PassDumpContext dump_context{api::PassDumpFormat::Text, &std::cout};
    pass->DumpAfterPass(dump_context);
  }
  if (graph_after_passes_.count(pass_name.as_string())) {
    api::PassDumpContext dump_context{api::PassDumpFormat::Graph, &std::cout};
    pass->DumpAfterPass(dump_context);
  }
}

void Compiler::DidStartPass(api::Pass* pass) {
  auto const pass_name = pass->name();
  DVLOG(0) << "Start: " << pass_name;
  if (dump_before_passes_.count(pass_name.as_string())) {
    api::PassDumpContext dump_context{api::PassDumpFormat::Text, &std::cout};
    pass->DumpBeforePass(dump_context);
  }
  if (graph_before_passes_.count(pass_name.as_string())) {
    api::PassDumpContext dump_context{api::PassDumpFormat::Graph, &std::cout};
    pass->DumpBeforePass(dump_context);
  }
}

}  // namespace shell
}  // namespace compiler
}  // namespace elang
