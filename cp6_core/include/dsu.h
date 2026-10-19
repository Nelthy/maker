#pragma once
#include "ab_link.h"
#include <vector>

namespace cp6 {
struct dsu {
  std::vector<unsigned> parent;
  std::vector<unsigned> size;

  dsu() = default;

  explicit dsu(std::size_t n) : parent(n), size(n, 1) {
    for (std::size_t i = 0; i < n; ++i)
      parent[i] = i;
  }

  inline unsigned find_root(unsigned x) {
    while (parent[x] != x) {
      parent[x] = parent[parent[x]]; // path halving
      x = parent[x];
    }
    return x;
  }

  inline void unite(unsigned x, unsigned y) {
    unsigned rx = find_root(x);
    unsigned ry = find_root(y);
    if (rx == ry) return;
    if (size[rx] < size[ry]) std::swap(rx, ry);
    parent[ry] = rx;
    size[rx] += size[ry];
  }
};

inline void find_graph_components(
  const std::vector<ab_link> &edges,
  std::vector<std::vector<unsigned>> &graph_islands,
  unsigned max_id,
  bool include_isolated
);
} // namespace cp6
