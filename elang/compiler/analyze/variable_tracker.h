// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_COMPILER_ANALYZE_VARIABLE_TRACKER_H_
#define ELANG_COMPILER_ANALYZE_VARIABLE_TRACKER_H_

#include <unordered_map>

#include "base/macros.h"
#include "elang/base/zone_user.h"
#include "elang/compiler/compilation_session_user.h"

namespace elang {
class Zone;
namespace compiler {

namespace ast {
class Method;
class NamedNode;
}

namespace ir {
class Factory;
}

namespace ts {
class Factory;
class Value;
}

//////////////////////////////////////////////////////////////////////
//
// VariableTracker
//
class VariableTracker final : public CompilationSessionUser, public ZoneUser {
 public:
  // |context_method| specified context where variables are used.
  VariableTracker(CompilationSession* session,
                  Zone* zone,
                  ast::Method* context_method);
  ~VariableTracker();

  void Finish(ir::Factory* factory, ts::Factory* type_factory);
  ts::Value* RecordGet(ast::NamedNode* variable);
  void RecordSet(ast::NamedNode* variable);
  void RegisterVariable(ast::NamedNode* variable, ts::Value* value);

 private:
  struct TrackingData;

  ast::Method* const context_method_;
  std::unordered_map<ast::NamedNode*, TrackingData*> variable_map_;

  DISALLOW_COPY_AND_ASSIGN(VariableTracker);
};

}  // namespace compiler
}  // namespace elang

#endif  // ELANG_COMPILER_ANALYZE_VARIABLE_TRACKER_H_
