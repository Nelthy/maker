#pragma once
#include "basic_types.h"

namespace dlm {
class size_ds {
public:
  static constexpr rank_t rank = 1;

  index_t size() const { return m_size; };

private:
  friend class ds_modifier;

  void swap(size_ds &other, index_t a, index_t b) {
    index_t temp = m_size;
    m_size = other.size();
    other.m_size = temp;
  }

  void move(size_ds &other, index_t dst, index_t src) { m_size = other.m_size; }

  void resize(index_t new_size) { m_size = new_size; }

  index_t m_size = 0;
};
} // namespace dlm
