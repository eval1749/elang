// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/hir/class.h"
#include "elang/hir/namespace.h"
#include "elang/hir/simple_name.h"

namespace elang {
namespace hir {

namespace {

Namespace* CreateGlobalNamespace(Factory* factory) {
  return factory->NewNamespace(nullptr, factory->GetOrCreateSimpleName(L"::"));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : global_namespace_(CreateGlobalNamespace(this)), temp_name_counter_(0) {
}

Factory::~Factory() {
  RemoveAll();
}

SimpleName* Factory::GetOrCreateSimpleName(base::StringPiece16 string) {
  auto const it = simple_names_.find(string);
  if (it != simple_names_.end())
    return it->second;
  auto const simple_name = new SimpleName(NewString(string));
  simple_names_[simple_name->string()] = simple_name;
  return simple_name;
}

Class* Factory::NewClass(Namespace* outer,
                         SimpleName* simple_name,
                         const std::vector<Class*>& base_classes) {
  auto const node = new Class(outer, simple_name, base_classes);
  nodes_.push_back(node);
  return node;
}

Namespace* Factory::NewNamespace(Namespace* outer, SimpleName* simple_name) {
  auto const node = new Namespace(outer, simple_name);
  nodes_.push_back(node);
  return node;
}

SimpleName* Factory::NewUniqueName(const base::char16* format) {
  return new SimpleName(base::StringPrintf(format, ++temp_name_counter_));
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string_piece) {
  auto const string =
      new base::string16(string_piece.data(), string_piece.size());
  strings_.push_back(string);
  return base::StringPiece16(string->data(), string->size());
}

void Factory::RemoveAll() {
  for (auto const node : nodes_)
    delete node;
  nodes_.clear();
  for (auto const string : strings_)
    delete string;
  strings_.clear();
}

}  // namespace hir
}  // namespace elang
