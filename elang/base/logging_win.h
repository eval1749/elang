// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_LOGGING_WIN_H_
#define ELANG_BASE_LOGGING_WIN_H_

#include "base/logging.h"

#define VERIFY_WIN32API(expr)                \
  {                                          \
    if (!(expr)) {                           \
      auto const error = ::GetLastError();   \
      LOG(0) << #expr << " error=" << error; \
    }                                        \
  }

#endif  // ELANG_BASE_LOGGING_WIN_H_
