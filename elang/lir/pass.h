// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_PASS_H_
#define ELANG_LIR_PASS_H_

#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "elang/api/pass.h"
#include "elang/lir/editor_user.h"
#include "elang/lir/factory_user.h"

namespace elang {
namespace lir {

class Function;

//////////////////////////////////////////////////////////////////////
//
// Pass
//
class ELANG_LIR_EXPORT Pass : public api::Pass, public FactoryUser {
 public:
  virtual ~Pass();

  virtual bool Run() = 0;

 protected:
  explicit Pass(Factory* factory);

 private:
  DISALLOW_COPY_AND_ASSIGN(Pass);
};

//////////////////////////////////////////////////////////////////////
//
// FunctionPass
//
class ELANG_LIR_EXPORT FunctionPass : public Pass, public EditorUser {
 public:
  virtual ~FunctionPass();

  // Pass
  bool Run() final;

 protected:
  explicit FunctionPass(Editor* editor);

  virtual void RunOnFunction() = 0;

  // api::Pass
  void DumpAfterPass(const api::PassDumpContext& context) override;
  void DumpBeforePass(const api::PassDumpContext& context) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(FunctionPass);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_PASS_H_
