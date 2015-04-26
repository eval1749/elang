// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <string>

#include "elang/optimizer/factory.h"

#include "base/logging.h"
#include "elang/base/atomic_string.h"
#include "elang/base/atomic_string_factory.h"
#include "elang/base/zone.h"
#include "elang/optimizer/error_data.h"
#include "elang/optimizer/function.h"
#include "elang/optimizer/nodes.h"
#include "elang/optimizer/node_factory.h"
#include "elang/optimizer/scheduler/schedule.h"
#include "elang/optimizer/scheduler/scheduler.h"
#include "elang/optimizer/types.h"
#include "elang/optimizer/type_factory.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// Factory
//
Factory::Factory(const FactoryConfig& config)
    : NodeFactoryUser(new NodeFactory(new TypeFactory(config))),
      TypeFactoryUser(node_factory()->type_factory()),
      atomic_string_factory_(config.atomic_string_factory),
      config_(config),
      last_function_id_(0),
      node_factory_(node_factory()),
      type_factory_(type_factory()) {
}

Factory::~Factory() {
}

std::unique_ptr<Schedule> Factory::ComputeSchedule(Function* function) {
  auto schedule = std::make_unique<Schedule>(function);
  Scheduler(schedule.get()).Run();
  return std::move(schedule);
}

AtomicString* Factory::NewAtomicString(base::StringPiece16 string) {
  return atomic_string_factory_->NewAtomicString(string);
}

Function* Factory::NewFunction(FunctionType* function_type) {
  auto const entry_node =
      node_factory()->NewEntry(function_type->parameters_type());
  auto const control = node_factory()->NewMerge({});
  auto const exit_node = node_factory()->NewExit(control);
  auto const function = new (zone()) Function(
      node_factory()->node_id_source(), function_type, entry_node, exit_node);
  function->id_ = ++last_function_id_;
  return function;
}

}  // namespace optimizer
}  // namespace elang
