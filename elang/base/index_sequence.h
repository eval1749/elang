// Copyright 2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_INDEX_SEQUENCE_H_
#define ELANG_BASE_INDEX_SEQUENCE_H_

namespace elang {

// Simple implementation of C++14 std::index_sequence<size_t ...>
template<size_t...> struct IndexSequence {};

template <size_t... Ns>
struct MakeIndexSequenceImpl;

template <size_t... Ns>
struct MakeIndexSequenceImpl<0, Ns...> {
  using Type = IndexSequence<Ns...>;
};

template <size_t N, size_t... Ns>
struct MakeIndexSequenceImpl<N, Ns...>
    : MakeIndexSequenceImpl<N - 1, N - 1, Ns...> {};

template <size_t N>
using MakeIndexSequence = typename MakeIndexSequenceImpl<N>::Type;

}  // namespace elang

#endif  // ELANG_BASE_INDEX_SEQUENCE_H_
