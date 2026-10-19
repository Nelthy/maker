#pragma once
#include "ds_modifier.h"
#include <vector>

namespace dlm {
template<typename DS>
class vec_bundle_ds {
public:
  static constexpr rank_t rank = DS::rank;

  auto &get(index_t i) { return m_data_sets[i]; }

  const auto &get(index_t i) const { return m_data_sets[i]; }

  template<rank_t I>
  auto &get() {
    return m_data_sets[I];
  }

  template<rank_t I>
  const auto &get() const {
    return m_data_sets[I];
  }

  template<rank_t RANK = 0>
  index_t size(ikey<RANK> key = {}) const {
    return ds_modifier::size(get<0>(), key);
  }

  template<typename T>
  void add_ds(T &&ds) {
    m_data_sets.push_back(std::forward<T>(ds));
  }

  index_t ds_num() const { return m_data_sets.size(); }

private:
  friend class ds_modifier;
  std::vector<DS> m_data_sets;

  template<rank_t RANK>
  void resize(index_t new_size, ikey<RANK> key) {
    for (rank_t i = 1; i < m_data_sets.size(); i++) {
      ds_modifier::resize(m_data_sets[i], new_size, key);
    }
    if (m_data_sets.size() != 0) {
      ds_modifier::resize(m_data_sets[0], new_size, key);
    }
  }

  template<rank_t RANK>
  void move(ikey<RANK> dst, ikey<RANK> src) {
    for (rank_t i = 0; i < m_data_sets.size(); i++) {
      ds_modifier::move(m_data_sets[i], dst, src);
    }
  }

  template<rank_t RANK>
  void swap(ikey<RANK> a, ikey<RANK> b) {
    for (rank_t i = 0; i < m_data_sets.size(); i++) {
      ds_modifier::swap(m_data_sets[i], a, b);
    }
  }

  template<rank_t RANK>
  void move(vec_bundle_ds &other, ikey<RANK> dst, ikey<RANK> src) {
    assert(false);
  }

  template<rank_t RANK>
  void swap(vec_bundle_ds &other, ikey<RANK> a, ikey<RANK> b) {
    assert(false);
  }

  template<rank_t RANK>
  void null(ikey<RANK> key) {
    for (rank_t i = 0; i < m_data_sets.size(); i++) {
      ds_modifier::null(m_data_sets[i], key);
    }
  }

  void clear_null() {
    for (rank_t i = 0; i < m_data_sets.size(); i++) {
      ds_modifier::clear_null(m_data_sets[i]);
    }
  }

  template<rank_t RANK>
  void merge(ikey<RANK> dst, ikey<RANK> src) {
    for (rank_t i = 0; i < m_data_sets.size(); i++) {
      ds_modifier::merge(m_data_sets[i], dst, src);
    }
  }
};
} // namespace dlm
