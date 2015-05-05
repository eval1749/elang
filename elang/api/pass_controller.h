// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_API_PASS_CONTROLLER_H_
#define ELANG_API_PASS_CONTROLLER_H_

#include "base/macros.h"

namespace elang {
namespace api {

class Pass;

//////////////////////////////////////////////////////////////////////
//
// PassController
//
class PassController {
 public:
  virtual ~PassController();

  virtual void DidEndPass(Pass* pass);
  virtual bool DidStartPass(Pass* pass);

 protected:
  PassController();

 private:
  DISALLOW_COPY_AND_ASSIGN(PassController);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_PASS_CONTROLLER_H_
