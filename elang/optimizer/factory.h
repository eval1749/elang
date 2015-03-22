// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_FACTORY_H_
#define ELANG_OPTIMIZER_FACTORY_H_

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "base/strings/string16.h"
#include "base/strings/string_piece.h"
#include "elang/base/zone_owner.h"
#include "elang/optimizer/error_sink.h"
#include "elang/optimizer/factory_config.h"
#include "elang/optimizer/node_factory_user.h"
#include "elang/optimizer/optimizer_export.h"
#include "elang/optimizer/type_factory_user.h"

namespace elang {
namespace optimizer {

class TypeFactory;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_OPTIMIZER_EXPORT Factory final : public ErrorSink,
                                             public NodeFactoryUser,
                                             public TypeFactoryUser {
 public:
  explicit Factory(const FactoryConfig& config);
  ~Factory();

  const FactoryConfig& config() const { return config_; }

  AtomicString* NewAtomicString(base::StringPiece16 string);

 private:
  AtomicStringFactory* const atomic_string_factory_;
  const FactoryConfig config_;

  std::unique_ptr<NodeFactory> node_factory_;
  std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FACTORY_H_
