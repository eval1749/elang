// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <array>
#include <iostream>
#include <string>

#include "elang/vm/machine_code_collection.h"

#include "base/logging.h"
#include "base/strings/string16.h"
#include "base/strings/utf_string_conversions.h"
#include "elang/vm/factory.h"
#include "elang/vm/machine_code_builder_impl.h"
#include "elang/vm/machine_code_function.h"
#include "elang/vm/objects.h"

namespace elang {
namespace vm {

namespace {
void ConsoleWriteLineString(impl::String* string) {
  auto const start =
      static_cast<base::char16*>(string->characters->first_element);
  auto const end = start + string->characters->length;
  for (auto ptr = start; ptr < end; ++ptr)
    std::cout << *ptr;
}

}  // namespace

MachineCodeCollection::MachineCodeCollection(Factory* factory)
    : factory_(factory) {
  InstallPredefinedFunction(
      factory, "System.Void System.Console.WriteLine(System.String)",
      reinterpret_cast<uintptr_t>(&ConsoleWriteLineString));
}

MachineCodeCollection::~MachineCodeCollection() {
}

MachineCodeFunction* MachineCodeCollection::FunctionByAddress(
    uintptr_t address) const {
  auto const it = address_map_.lower_bound(address);
  if (it == address_map_.end())
    return nullptr;
  auto const function = it->second;
  DCHECK_GE(address, function->address());
  if (address - function->address() >= function->code_size())
    return nullptr;
  return function;
}

MachineCodeFunction* MachineCodeCollection::FunctionByName(
    AtomicString* name) const {
  auto const it = name_map_.find(name);
  return it == name_map_.end() ? nullptr : it->second;
}

void MachineCodeCollection::InstallPredefinedFunction(Factory* factory,
                                                      base::StringPiece name,
                                                      uintptr_t entry_point) {
  MachineCodeBuilderImpl builder_impl(factory);
  api::MachineCodeBuilder* builder = &builder_impl;
  std::array<uint8_t, 2 + 4 + 2 + 8> code_buffer{
      0xFF,  // FF /4 JMP [RIP+2]
      0x25,  // ModRm(00, 100, 101)
      0x02,  // Disp32
      0x00,
      0x00,
      0x00,
      0x90,  // NOP
      0x90,  // NOP
  };
  code_buffer[8] = static_cast<uint8_t>(entry_point);
  code_buffer[9] = static_cast<uint8_t>(entry_point >> 8);
  code_buffer[10] = static_cast<uint8_t>(entry_point >> 16);
  code_buffer[11] = static_cast<uint8_t>(entry_point >> 24);
  code_buffer[12] = static_cast<uint8_t>(entry_point >> 32);
  code_buffer[13] = static_cast<uint8_t>(entry_point >> 40);
  code_buffer[14] = static_cast<uint8_t>(entry_point >> 48);
  code_buffer[15] = static_cast<uint8_t>(entry_point >> 56);
  auto const code_size = code_buffer.size();
  builder->PrepareCode(code_size);
  builder->EmitCode(code_buffer.data(), code_size);
  builder->FinishCode();
  auto const key = factory_->NewAtomicString(base::UTF8ToUTF16(name));
  RegisterFunction(key, builder_impl.NewMachineCodeFunction());
}

void MachineCodeCollection::RegisterFunction(AtomicString* name,
                                             MachineCodeFunction* function) {
  DCHECK(!FunctionByAddress(function->address()));
  address_map_[function->address()] = function;
  if (!name)
    return;
  DCHECK(!name_map_.count(name));
  name_map_[name] = function;
}

}  // namespace vm
}  // namespace elang
