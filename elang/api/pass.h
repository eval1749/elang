// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_API_PASS_H_
#define ELANG_API_PASS_H_

#include <iosfwd>  // NOLINT
#include <string>

#include "base/macros.h"
#include "base/strings/string_piece.h"
#include "base/time/time.h"

namespace elang {
namespace api {

class PassObserver;

struct PassDumpContext {
  std::ostream* ostream;
};

//////////////////////////////////////////////////////////////////////
//
// Pass
//
class Pass {
 public:
  virtual ~Pass();

  base::TimeDelta duration() const;
  virtual base::StringPiece name() const = 0;

  virtual void DumpAfterPass(const PassDumpContext& context);
  virtual void DumpBeforePass(const PassDumpContext& context);

 protected:
  class RunScope final {
   public:
    explicit RunScope(Pass* pass);
    ~RunScope();

   private:
    Pass* const pass_;

    DISALLOW_COPY_AND_ASSIGN(RunScope);
  };

  explicit Pass(PassObserver* observer);

  PassObserver* observer() const { return observer_; }

  void EndPass();
  void StartPass();

 private:
  base::Time end_at_;
  PassObserver* const observer_;
  base::Time start_at_;

  DISALLOW_COPY_AND_ASSIGN(Pass);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_PASS_H_
