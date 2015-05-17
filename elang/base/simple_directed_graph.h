// Copyright 2014-2015 Project Vogue. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_
#define ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_

#include <algorithm>
#include <stack>
#include <unordered_set>
#include <vector>

#include "base/logging.h"
#include "base/macros.h"
#include "elang/base/zone_allocated.h"
#include "elang/base/zone_owner.h"
#include "elang/base/zone_unordered_map.h"
#include "elang/base/zone_vector.h"

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
  // Returns all vertices used so far including vertices without in/out edges.
  std::vector<T> GetAllVertices() const;
  std::vector<T> GetInEdges(const T& to) const;
  std::vector<T> GetOutEdges(const T& from) const;
  bool HasEdge(const T& from, const T& to) const;
  bool HasInEdge(const T& data) const;
  bool HasOutEdge(const T& data) const;
  void RemoveEdge(const T& from, const T& to);

 private:
  struct Vertex : ZoneAllocated {
    ZoneVector<Vertex*> ins;
    ZoneVector<Vertex*> outs;
    T data;
    Vertex(Zone* zone, const T& data) : data(data), ins(zone), outs(zone) {}
  };

  Vertex* GetOrNewVertex(const T& data) const {
    return const_cast<SimpleDirectedGraph*>(this)->GetOrNewVertex(data);
  }
  Vertex* GetOrNewVertex(const T& data);

  mutable ZoneUnorderedMap<T, Vertex*> vertex_map_;

  DISALLOW_COPY_AND_ASSIGN(SimpleDirectedGraph);
};

template <typename T>
void SimpleDirectedGraph<T>::AddEdge(const T& from, const T& to) {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);
  if (HasEdge(from, to))
    return;
  from_vertex->outs.push_back(to_vertex);
  to_vertex->ins.push_back(from_vertex);
}

template <typename T>
std::vector<T> SimpleDirectedGraph<T>::GetAllVertices() const {
  std::vector<T> vertices(vertex_map_.size());
  vertices.resize(0);
  for (const auto& pair : vertex_map_)
    vertices.push_back(pair.first);
  return vertices;
}

template <typename T>
std::vector<T> SimpleDirectedGraph<T>::GetInEdges(const T& data) const {
  auto const vertex = GetOrNewVertex(data);
  std::vector<T> from_list(vertex->ins.size());
  from_list.resize(0);
  for (auto const from_vertex : vertex->ins)
    from_list.push_back(from_vertex->data);
  return std::move(from_list);
}

template <typename T>
std::vector<T> SimpleDirectedGraph<T>::GetOutEdges(const T& data) const {
  auto const vertex = GetOrNewVertex(data);
  std::vector<T> to_list(vertex->outs.size());
  to_list.resize(0);
  for (auto const to_vertex : vertex->outs)
    to_list.push_back(to_vertex->data);
  return std::move(to_list);
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
bool SimpleDirectedGraph<T>::HasEdge(const T& from, const T& to) const {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);
  auto const it =
      std::find(from_vertex->outs.begin(), from_vertex->outs.end(), to_vertex);
  return it != from_vertex->outs.end();
}

template <typename T>
bool SimpleDirectedGraph<T>::HasInEdge(const T& data) const {
  auto const vertex = GetOrNewVertex(data);
  return !vertex->ins.empty();
}

template <typename T>
bool SimpleDirectedGraph<T>::HasOutEdge(const T& data) const {
  auto const vertex = GetOrNewVertex(data);
  return !vertex->outs.empty();
}

template <typename T>
void SimpleDirectedGraph<T>::RemoveEdge(const T& from, const T& to) {
  auto const from_vertex = GetOrNewVertex(from);
  auto const to_vertex = GetOrNewVertex(to);

  auto const from_it =
      std::find(from_vertex->outs.begin(), from_vertex->outs.end(), to_vertex);
  DCHECK(from_it != from_vertex->outs.end());
  from_vertex->outs.erase(from_it);

  auto const to_it =
      std::find(to_vertex->ins.begin(), to_vertex->ins.end(), from_vertex);
  DCHECK(to_it != to_vertex->ins.end());
  to_vertex->ins.erase(to_it);
}

}  // namespace elang

#endif  // ELANG_BASE_SIMPLE_DIRECTED_GRAPH_H_
