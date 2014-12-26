// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "elang/compiler/qualified_name.h"

#include "base/logging.h"

namespace elang {
namespace compiler {

//////////////////////////////////////////////////////////////////////
//
// QualifiedName
//
QualifiedName::QualifiedName(const std::vector<Token*>& simple_names)
    : simple_names_(simple_names) {
#if DEBUG
  for (const auto& name : simple_names_)
    DCHECK(name.is_name());
#endif
}

QualifiedName::QualifiedName(const QualifiedName& other)
    : simple_names_(other.simple_names_) {
}

QualifiedName::QualifiedName(Token* simple_name)
    : simple_names_(std::vector<Token*> { simple_name }) {
  DCHECK(simple_name->is_name());
}

QualifiedName::QualifiedName(QualifiedName&& other)
    : simple_names_(std::move(other.simple_names_)) {
}

QualifiedName::~QualifiedName() {
}

QualifiedName& QualifiedName::operator=(const QualifiedName& other) {
  simple_names_ = other.simple_names_;
  return *this;
}

QualifiedName& QualifiedName::operator=(QualifiedName&& other) {
  simple_names_ = std::move(other.simple_names_);
  return *this;
}

}  // namespace compiler
}  // namespace elang
