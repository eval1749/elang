// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/lir/error_sink.h"

#include "elang/lir/error_data.h"

namespace elang {
namespace lir {

//////////////////////////////////////////////////////////////////////
//
// ErrorSink
//
ErrorSink::ErrorSink(Zone* zone, LiteralMap* literals)
    : ZoneUser(zone), literals_(literals) {
}

ErrorSink::~ErrorSink() {
}

void ErrorSink::AddError(ErrorCode error_code,
                         Value value,
                         const std::vector<Value>& details) {
  errors_.push_back(
      new (zone()) ErrorData(zone(), literals_, error_code, value, details));
}

}  // namespace lir
}  // namespace elang
