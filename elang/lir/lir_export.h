// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_LIR_LIR_EXPORT_H_
#define ELANG_LIR_LIR_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(LIR_IMPLEMENTATION)
#define ELANG_LIR_EXPORT __declspec(dllexport)
#else
#define ELANG_LIR_EXPORT __declspec(dllimport)
#endif  // defined(LIR_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(LIR_IMPLEMENTATION)
#define ELANG_LIR_EXPORT __attribute__((visibility("default")))
#else
#define ELANG_LIR_EXPORT
#endif  // defined(LIR_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define ELANG_LIR_EXPORT
#endif

// TODO(eval1749) Move disabling C4275 to ".gn" file.
// C4275 non – DLL-interface classkey 'identifier' used as base for
// DLL-interface classkey 'identifier'
#pragma warning(disable : 4275)

#endif  // ELANG_LIR_LIR_EXPORT_H_
