// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/vm/factory.h"

#include "base/logging.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/vm/class.h"
#include "elang/vm/namespace.h"

namespace elang {
namespace vm {

namespace {

Namespace* CreateGlobalNamespace(Factory* factory) {
  return factory->NewNamespace(nullptr, factory->NewAtomicString(L"."));
}

}  // namespace

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory()
    : atomic_string_factory_(new AtomicStringFactory()),
      zone_(new Zone()),
      global_namespace_(CreateGlobalNamespace(this)) {
}

Factory::~Factory() {
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

Class* Factory::NewClass(Namespace* outer,
                         AtomicString* name,
                         const std::vector<Class*>& base_classes) {
  return new (zone_.get()) Class(zone_.get(), outer, name, base_classes);
}

Namespace* Factory::NewNamespace(Namespace* outer, AtomicString* name) {
  return new (zone_.get()) Namespace(zone_.get(), outer, name);
}

base::StringPiece16 Factory::NewString(base::StringPiece16 string) {
  return atomic_string_factory_->NewString(string);
}

}  // namespace vm
}  // namespace elang
