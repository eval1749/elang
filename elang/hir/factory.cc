// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/hir/factory.h"

#include "base/logging.h"
#include "base/strings/stringprintf.h"
#include "elang/base/zone.h"
#include "elang/hir/atomic_string.h"
#include "elang/hir/class.h"
#include "elang/hir/namespace.h"

namespace elang {
namespace hir {

namespace {

Namespace* CreateGlobalNamespace(Factory* factory) {
  return factory->NewNamespace(nullptr,
                               factory->GetOrCreateAtomicString(L"::"));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(Zone* zone)
    : zone_(zone),
      global_namespace_(CreateGlobalNamespace(this)),
      temp_name_counter_(0) {
}

Factory::~Factory() {
  RemoveAll();
}

AtomicString* Factory::GetOrCreateAtomicString(base::StringPiece16 string) {
  auto const it = simple_names_.find(string);
  if (it != simple_names_.end())
    return it->second;
  auto const simple_name = new (zone_) AtomicString(NewString(string));
  simple_names_[simple_name->string()] = simple_name;
  return simple_name;
}

Class* Factory::NewClass(Namespace* outer,
                         AtomicString* simple_name,
                         const std::vector<Class*>& base_classes) {
  auto const node = new Class(outer, simple_name, base_classes);
  nodes_.push_back(node);
  return node;
}

Namespace* Factory::NewNamespace(Namespace* outer, AtomicString* simple_name) {
  auto const node = new Namespace(outer, simple_name);
  nodes_.push_back(node);
  return node;
}

AtomicString* Factory::NewUniqueName(const base::char16* format) {
  return new (zone_)
      AtomicString(base::StringPrintf(format, ++temp_name_counter_));
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string_piece) {
  auto const size = string_piece.size() * sizeof(base::char16);
  auto const string = static_cast<base::char16*>(zone_->Allocate(size));
  ::memcpy(string, string_piece.data(), size);
  return base::StringPiece16(string, string_piece.size());
}

void Factory::RemoveAll() {
  for (auto const node : nodes_)
    delete node;
  nodes_.clear();
}

}  // namespace hir
}  // namespace elang
