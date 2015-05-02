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

enum class PassDumpFormat {
  Graph,
  Html,
  Text,
};

struct PassDumpContext {
  PassDumpFormat format;
  std::ostream* ostream;

  bool IsGraph() const { return format == PassDumpFormat::Graph; }
  bool IsHtml() const { return format == PassDumpFormat::Html; }
  bool IsText() const { return format == PassDumpFormat::Text; }
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

    bool IsStop() const { return stop_; }

   private:
    Pass* const pass_;
    bool const stop_;

    DISALLOW_COPY_AND_ASSIGN(RunScope);
  };

  explicit Pass(PassObserver* observer);

  PassObserver* observer() const { return observer_; }

  void EndPass();
  bool StartPass();

 private:
  base::Time end_at_;
  PassObserver* const observer_;
  base::Time start_at_;

  DISALLOW_COPY_AND_ASSIGN(Pass);
};

}  // namespace api
}  // namespace elang

#endif  // ELANG_API_PASS_H_
