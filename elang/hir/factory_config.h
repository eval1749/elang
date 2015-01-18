// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_FACTORY_CONFIG_H_
#define ELANG_HIR_FACTORY_CONFIG_H_

namespace elang {
class AtomicString;
class AtomicStringFactory;
namespace hir {

//////////////////////////////////////////////////////////////////////
//
// FactoryConfig
//
struct FactoryConfig {
  AtomicStringFactory* atomic_string_factory;
  AtomicString* string_type_name;
};

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_FACTORY_CONFIG_H_
