// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "elang/optimizer/error_reporter.h"

#include "elang/optimizer/error_data.h"
#include "elang/optimizer/factory.h"

namespace elang {
namespace optimizer {

ErrorReporter::ErrorReporter(Factory* factory) : factory_(factory) {
}

ErrorReporter::~ErrorReporter() {
}

void ErrorReporter::Error(ErrorCode error_code, Node* error_node) {
  factory_->AddError(error_code, error_node, {});
}

void ErrorReporter::Error(ErrorCode error_code,
                          Node* error_node,
                          Thing* detail) {
  factory_->AddError(error_code, error_node, {detail});
}

void ErrorReporter::Error(ErrorCode error_code,
                          Node* error_node,
                          Thing* detail1,
                          Thing* detail2) {
  factory_->AddError(error_code, error_node, {detail1, detail2});
}

}  // namespace optimizer
}  // namespace elang
