// Copyright 2014 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_
#define ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_

#include <vector>

#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_unordered_set.h"

namespace elang {

//////////////////////////////////////////////////////////////////////
//
// Represents directed  graph. It is OK to have cycle.
//
template <typename T>
class SimpleDirectedGraph : public ZoneOwner {
 public:
  SimpleDirectedGraph() : vertex_map_(zone()) {}
  ~SimpleDirectedGraph() = default;

  void AddEdge(const T& from, const T& to);
  std::vector<T> GetInEdges(const T& to);
  std::vector<T> GetOutEdges(const T& from);
  bool HasEdge(const T& from, const T& to);
  bool HasInEdge(const T& data);
  bool HasOutEdge(const T& data);
  void RemoveEdge(const T& from, const T& to);

 private:
  struct Vertex : ZoneAllocated {
    ZoneUnorderedSet<Vertex*> ins;
    ZoneUnorderedSet<Vertex*> outs;
    T data;
    Vertex(Zone* zone, const T& data) : data(data), ins(zone), outs(zone) {}
  };

  Vertex* GetOrNewVertex(const T& data);

  ZoneUnorderedMap<T, Vertex*> vertex_map_;

  DISALLOW_COPY_AND_ASSIGN(SimpleDirectedGraph);
};

template <typename T>
void SimpleDirectedGraph<T>::AddEdge(const T& from, const T& to) {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);
  from_vertex->outs.insert(to_vertex);
  to_vertex->ins.insert(from_vertex);
}

template <typename T>
typename SimpleDirectedGraph<T>::Vertex* SimpleDirectedGraph<T>::GetOrNewVertex(
    const T& data) {
  auto const it = vertex_map_.find(data);
  if (it != vertex_map_.end())
    return it->second;
  auto const new_vertex = new (zone()) Vertex(zone(), data);
  vertex_map_[data] = new_vertex;
  return new_vertex;
}

template <typename T>
std::vector<T> SimpleDirectedGraph<T>::GetInEdges(const T& data) {
  auto const vertex = GetOrNewVertex(data);
  std::vector<T> ins(vertex->ins.size());
  ins.resize(0);
  for (const auto& in : vertex->ins)
    ins.push_back(in->data);
  return ins;
}

template <typename T>
std::vector<T> SimpleDirectedGraph<T>::GetOutEdges(const T& data) {
  auto const vertex = GetOrNewVertex(data);
  std::vector<T> outs(vertex->outs.size());
  outs.resize(0);
  for (const auto& out : vertex->outs)
    outs.push_back(out->data);
  return outs;
}

template <typename T>
bool SimpleDirectedGraph<T>::HasEdge(const T& from, const T& to) {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);
  return from_vertex->outs.count(to_vertex);
}

template <typename T>
bool SimpleDirectedGraph<T>::HasInEdge(const T& data) {
  auto const vertex = GetOrNewVertex(data);
  return !vertex->ins.empty();
}

template <typename T>
bool SimpleDirectedGraph<T>::HasOutEdge(const T& data) {
  auto const vertex = GetOrNewVertex(data);
  return !vertex->outs.empty();
}

template <typename T>
void SimpleDirectedGraph<T>::RemoveEdge(const T& from, const T& to) {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);
  from_vertex->outs.erase(to_vertex);
  to_vertex->ins.erase(from_vertex);
}

}  // namespace elang

#endif  // ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_