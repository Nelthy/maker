#pragma once
#include "cassert"
#include <cstddef>
#include <vector>

class bin_table {
public:
  inline auto get(unsigned r, unsigned c) const {
    assert(size_t{r} * size_t{m_c_num} + size_t{c} < m_data.size());
    return m_data[size_t{r} * size_t{m_c_num} + size_t{c}];
  }

  inline void set(unsigned r, unsigned c, bool value) {
    assert(size_t{r} * size_t{m_c_num} + size_t{c} < m_data.size());
    m_data[size_t{r} * size_t{m_c_num} + size_t{c}] = value;
  }

  inline void flip(unsigned r, unsigned c) {
    assert(size_t{r} * size_t{m_c_num} + size_t{c} < m_data.size());
    m_data[size_t{r} * size_t{m_c_num} + size_t{c}].flip();
  }

  inline bool clear() { m_data.clear(); return true; }

  inline bool resize(unsigned r, unsigned c) {
    m_data.resize(size_t{r} * size_t{c});
    m_c_num = c;
    return true;
  }

private:
  std::vector<bool> m_data;
  unsigned m_c_num;
};
