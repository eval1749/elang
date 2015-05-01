// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_API_PASS_OBSERVER_H_
#define ELANG_API_PASS_OBSERVER_H_

#include "base/macros.h"

namespace elang {
namespace api {

class Pass;

//////////////////////////////////////////////////////////////////////
//
// PassObserver
//
class PassObserver {
 public:
  virtual ~PassObserver();

  virtual void DidEndPass(Pass* pass) = 0;
  virtual void DidStartPass(Pass* pass) = 0;

 protected:
  PassObserver();

 private:
  DISALLOW_COPY_AND_ASSIGN(PassObserver);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_PASS_OBSERVER_H_
