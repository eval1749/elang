// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_EDITOR_OWNER_H_
#define ELANG_LIR_EDITOR_OWNER_H_

#include <memory>

#include "base/macros.h"
#include "elang/lir/editor_user.h"

namespace elang {
namespace lir {

class Editor;
class Factory;
class Function;

//////////////////////////////////////////////////////////////////////
//
// EditorOwner
//
class ELANG_LIR_EXPORT EditorOwner : public EditorUser {
 protected:
  EditorOwner(Factory* factory, Function* function);
  ~EditorOwner();

 private:
  const std::unique_ptr<Editor> editor_;

  DISALLOW_COPY_AND_ASSIGN(EditorOwner);
};

}  // namespace lir
}  // namespace elang

#endif  // ELANG_LIR_EDITOR_OWNER_H_
