// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_BASE_EXPORT_H_
#define ELANG_BASE_BASE_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(ELANG_BASE_IMPLEMENTATION)
#define ELANG_BASE_EXPORT __declspec(dllexport)
#else
#define ELANG_BASE_EXPORT __declspec(dllimport)
#endif  // defined(BASE_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(ELANG_BASE_IMPLEMENTATION)
#define ELANG_BASE_EXPORT __attribute__((visibility("default")))
#else
#define ELANG_BASE_EXPORT
#endif  // defined(BASE_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define ELANG_BASE_EXPORT
#endif

#endif  // ELANG_BASE_BASE_EXPORT_H_
