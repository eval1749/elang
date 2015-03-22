// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/optimizer/error_sink.h"

#include "elang/optimizer/error_data.h"

namespace elang {
namespace optimizer {

//////////////////////////////////////////////////////////////////////
//
// ErrorSink
//
ErrorSink::ErrorSink() {
}

ErrorSink::~ErrorSink() {
}

void ErrorSink::AddError(ErrorCode error_code,
                         const Node* node,
                         const std::vector<Thing*>& details) {
  errors_.push_back(new (zone()) ErrorData(zone(), error_code,
                                           const_cast<Node*>(node), details));
}

}  // namespace optimizer
}  // namespace elang
