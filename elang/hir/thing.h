// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_THING_H_
#define ELANG_HIR_THING_H_

#include <ostream>
#include <vector>

#include "base/macros.h"
#include "elang/base/castable.h"
#include "elang/base/zone_allocated.h"
#include "elang/hir/hir_export.h"

namespace elang {
namespace hir {

// A base class of |Type| and |Value| for using them in |ErrorData|.
class ELANG_HIR_EXPORT Thing : public Castable<Thing>, public ZoneAllocated {
  DECLARE_CASTABLE_CLASS(Thing, Castable);

 protected:
  Thing() = default;
  ~Thing() override = default;

 private:
  DISALLOW_COPY_AND_ASSIGN(Thing);
};

// See "formatting/text_formatter.cc" for implementation of |Thing| printer.
ELANG_HIR_EXPORT std::ostream& operator<<(std::ostream& ostream,
                                          const Thing& thing);

}  // namespace hir
}  // namespace elang

#endif  // ELANG_HIR_THING_H_
