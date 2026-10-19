#pragma once
#include "tri_utils.h"
#include <cassert>
#include <cstddef>
#include <vector>

class tri_bin_table {
public:
  auto get(unsigned r, unsigned c) const {
    assert(tri_linearize(r, c, m_dim) < m_data.size());
    return m_data[tri_linearize(r, c, m_dim)];
  }

  void set(unsigned r, unsigned c, bool value) {
    assert(tri_linearize(r, c, m_dim) < m_data.size());
    m_data[tri_linearize(r, c, m_dim)] = value;
  }

  void flip(unsigned r, unsigned c) {
    assert(tri_linearize(r, c, m_dim) < m_data.size());
    m_data[tri_linearize(r, c, m_dim)].flip();
  }

  bool clear() {
    m_data.clear();
    m_dim = 0;
    return true;
  }

  bool resize(unsigned new_dim) {
    m_data.resize(tri_linearize(new_dim - 1, new_dim - 1, new_dim) + 1);
    m_dim = new_dim;
    return true;
  }

private:
  std::vector<bool> m_data;
  unsigned m_dim;
};
