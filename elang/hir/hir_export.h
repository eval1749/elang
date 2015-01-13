// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_HIR_HIR_EXPORT_H_
#define ELANG_HIR_HIR_EXPORT_H_

#if defined(COMPONENT_BUILD)
#if defined(WIN32)

#if defined(HIR_IMPLEMENTATION)
#define ELANG_HIR_EXPORT __declspec(dllexport)
#else
#define ELANG_HIR_EXPORT __declspec(dllimport)
#endif  // defined(HIR_IMPLEMENTATION)

#else  // defined(WIN32)
#if defined(HIR_IMPLEMENTATION)
#define ELANG_HIR_EXPORT __attribute__((visibility("default")))
#else
#define ELANG_HIR_EXPORT
#endif  // defined(HIR_IMPLEMENTATION)
#endif

#else  // defined(COMPONENT_BUILD)
#define ELANG_HIR_EXPORT
#endif

// TODO(eval1749) Move disabling C4275 to ".gn" file.
// C4275 non – DLL-interface classkey 'identifier' used as base for
// DLL-interface classkey 'identifier'
#pragma warning(disable : 4275)

#endif  // ELANG_HIR_HIR_EXPORT_H_
