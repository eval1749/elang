// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_OPTIMIZER_OPTIMIZER_EXPORT_H_
#define ELANG_OPTIMIZER_OPTIMIZER_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(OPTIMIZER_IMPLEMENTATION)
#define ELANG_OPTIMIZER_EXPORT __declspec(dllexport)
#else
#define ELANG_OPTIMIZER_EXPORT __declspec(dllimport)
#endif  // defined(OPTIMIZER_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(OPTIMIZER_IMPLEMENTATION)
#define ELANG_OPTIMIZER_EXPORT __attribute__((visibility("default")))
#else
#define ELANG_OPTIMIZER_EXPORT
#endif  // defined(OPTIMIZER_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define ELANG_OPTIMIZER_EXPORT
#endif

// TODO(eval1749) Move disabling C4275 to ".gn" file.
// C4275 non – DLL-interface classkey 'identifier' used as base for
// DLL-interface classkey 'identifier'
#pragma warning(disable : 4275)

#endif  // ELANG_OPTIMIZER_OPTIMIZER_EXPORT_H_
