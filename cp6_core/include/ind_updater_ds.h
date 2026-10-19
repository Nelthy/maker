#pragma once
#include "basic_types.h"
#include <vector>

// an indirection from the current configuration to the configuration at start.
struct ind_updater {

  const unsigned *get_ind() const { return m_ind.data(); }

  unsigned ind_size() const { return m_ind.size(); }

  void start() {
    if (not_identity) {
      m_ind.resize(m_new_size);
      for (unsigned i = 0; i < m_ind.size(); i++) {
        m_ind[i] = i;
      }
    }
  }

  unsigned size() const { return m_new_size; }

private:
  std::vector<unsigned> m_ind;
  unsigned m_new_size;
  bool not_identity = false;

  void swap(ind_updater &other, index_t a, index_t b) {
    std::swap(m_ind[a], other.m_ind[b]);
    not_identity = true;
  }

  void move(ind_updater &other, index_t dst, index_t src) {
    m_ind[dst] = std::move(other.m_ind[src]);
    not_identity = true;
  }

  void copy(const ind_updater &src_ds, index_t dst, index_t src) {
    m_ind[dst] = src_ds.m_ind[src];
    not_identity = true;
  }

  void resize(index_t new_size) {
    if (new_size > ind_size()) {
      for (unsigned i = ind_size(); i < new_size; i++) {
        m_ind[i] = -1;
      }
      m_new_size = new_size;
    } else {
      m_new_size = new_size;
      not_identity = true;
    }
  }
};
