// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_VM_FACTORY_H_
#define ELANG_VM_FACTORY_H_

#include <memory>
#include <string>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/vm/entry_point.h"

namespace elang {
class AtomicString;
class AtomicStringFactory;
class Zone;

namespace vm {
class Class;
class MemoryPool;
class Namespace;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class Factory final {
 public:
  Factory();
  ~Factory();

  Namespace* global_namespace() const { return global_namespace_; }
  Zone* zone() const { return zone_.get(); }

  AtomicString* NewAtomicString(base::StringPiece16 string);
  Class* NewClass(Namespace* outer,
                  AtomicString* simple_name,
                  const std::vector<Class*>& base_classes);
  EntryPoint NewCodeBlob(int size);
  void* NewDataBlob(int size);
  Namespace* NewNamespace(Namespace* outer, AtomicString* simple_name);
  base::StringPiece16 NewString(base::StringPiece16 string);

 private:
  const std::unique_ptr<MemoryPool> code_memory_pool_;
  const std::unique_ptr<MemoryPool> data_memory_pool_;
  const std::unique_ptr<AtomicStringFactory> atomic_string_factory_;
  const std::unique_ptr<Zone> zone_;

  // |global_namespace_| must be initialized after |zone_|.
  Namespace* const global_namespace_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace vm
}  // namespace elang

#endif  // ELANG_VM_FACTORY_H_
