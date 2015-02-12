// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EDITOR_USER_H_
#define ELANG_LIR_EDITOR_USER_H_

#include "base/macros.h"
#include "elang/lir/lir_export.h"

namespace elang {
namespace lir {

class Editor;
class Function;

//////////////////////////////////////////////////////////////////////
//
// EditorUser
//
class ELANG_LIR_EXPORT EditorUser {
 protected:
  explicit EditorUser(Editor* editor);
  ~EditorUser();

  Editor* editor() const { return editor_; }
  Function* function() const;

 private:
  Editor* const editor_;

  DISALLOW_COPY_AND_ASSIGN(EditorUser);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EDITOR_USER_H_
