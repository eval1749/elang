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
namespace api {
class PassController;
}
namespace optimizer {

class Function;
class Schedule;

//////////////////////////////////////////////////////////////////////
//
// Factory
//
class ELANG_OPTIMIZER_EXPORT Factory final : public ErrorSink,
                                             public NodeFactoryUser,
                                             public TypeFactoryUser {
 public:
  Factory(api::PassController* pass_controller, const FactoryConfig& config);
  ~Factory();

  const FactoryConfig& config() const { return config_; }
  api::PassController* pass_controller() const { return pass_controller_; }

  std::unique_ptr<Schedule> ComputeSchedule(Function* function);
  AtomicString* NewAtomicString(base::StringPiece16 string);
  Function* NewFunction(FunctionType* function_type);
  bool Optimize(Function* function, int level);

 private:
  AtomicStringFactory* const atomic_string_factory_;
  const FactoryConfig config_;
  size_t last_function_id_;
  const std::unique_ptr<NodeFactory> node_factory_;
  api::PassController* const pass_controller_;
  const std::unique_ptr<TypeFactory> type_factory_;

  DISALLOW_COPY_AND_ASSIGN(Factory);
};

}  // namespace optimizer
}  // namespace elang

#endif  // ELANG_OPTIMIZER_FACTORY_H_
