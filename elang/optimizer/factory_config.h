// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FACTORY_CONFIG_H_
#define ELANG_OPTIMIZER_FACTORY_CONFIG_H_

namespace elang {
class AtomicString;
class AtomicStringFactory;
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// FactoryConfig
//
struct FactoryConfig {
  AtomicStringFactory* atomic_string_factory;
  AtomicString* string_type_name;
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FACTORY_CONFIG_H_
