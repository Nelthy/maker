#include "dsu.h"
#include <cassert>

namespace cp6 {
void find_graph_components(
  const std::vector<ab_link> &edges,
  std::vector<std::vector<unsigned>> &graph_islands,
  unsigned max_id,
  bool include_isolated
) {
  graph_islands.clear();

  const std::size_t n = static_cast<std::size_t>(max_id) + 1;
  if (n == 0) return;

  dsu dsu(n);
  // mark nodes that actually appear in edges (optional)
  std::vector<char> has_edge(n, 0);

  // union edges
  for (const auto &e : edges) {
    // if edges might contain ids > max_id, either clamp or assert; here we assert.
    assert(e.a <= max_id && e.b <= max_id);
    if (e.a == e.b) {
      has_edge[e.a] = 1;
      continue;
    } // self-loop: mark node but skip unite
    has_edge[e.a] = 1;
    has_edge[e.b] = 1;
    dsu.unite(static_cast<unsigned>(e.a), static_cast<unsigned>(e.b));
  }

  // compute root for each index (also does final compression via find_root)
  std::vector<unsigned> root_of(n);
  for (std::size_t i = 0; i < n; ++i)
    root_of[i] = dsu.find_root(static_cast<unsigned>(i));

  // count component sizes but only for nodes we want to include
  std::vector<unsigned> comp_size(n, 0);
  unsigned nonzero_roots = 0;
  for (std::size_t i = 0; i < n; ++i) {
    if (!include_isolated && !has_edge[i]) continue;
    unsigned r = root_of[i];
    if (comp_size[r] == 0) ++nonzero_roots;
    ++comp_size[r];
  }

  // create containers and reserve space (one pass to avoid repeated reallocs)
  graph_islands.reserve(static_cast<std::size_t>(nonzero_roots));
  std::vector<unsigned> root_to_comp_idx(n, -1);
  for (std::size_t r = 0; r < n; ++r) {
    if (comp_size[r] > 0) {
      root_to_comp_idx[r] = static_cast<unsigned>(graph_islands.size());
      graph_islands.emplace_back();
      graph_islands.back().reserve(static_cast<std::size_t>(comp_size[r]));
    }
  }

  // fill components; nodes come out in increasing node id order (deterministic)
  for (std::size_t i = 0; i < n; ++i) {
    if (!include_isolated && !has_edge[i]) continue;
    unsigned r = root_of[i];
    unsigned comp_idx = root_to_comp_idx[r];
    graph_islands[static_cast<std::size_t>(comp_idx)].push_back(i);
  }
}
} // namespace cp6
