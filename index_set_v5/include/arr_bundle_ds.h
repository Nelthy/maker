#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include <array>

namespace dlm {
template<rank_t DS_NUM, typename DS>
  requires(DS_NUM > 0)
class arr_bundle_ds {
public:
  static constexpr rank_t rank = DS::rank;
  static constexpr rank_t ds_num = DS_NUM;

  auto &get(index_t i) { return m_data_sets[i]; }

  const auto &get(index_t i) const { return m_data_sets[i]; }

  template<rank_t I>
  auto &get() {
    return std::get<I>(m_data_sets);
  }

  template<rank_t I>
  const auto &get() const {
    return std::get<I>(m_data_sets);
  }

  template<rank_t RANK = 0>
  index_t size(ikey<RANK> key = {}) const {
    return ds_modifier::size(get<0>(), key);
  }

private:
  friend class ds_modifier;

  template<rank_t RANK>
  void resize(index_t new_size, ikey<RANK> key) {
    for (rank_t i = 1; i < ds_num; i++) {
      ds_modifier::resize(m_data_sets[i], new_size, key);
    }
    ds_modifier::resize(m_data_sets[0], new_size, key);
  }

  template<rank_t RANK>
  void move(ikey<RANK> dst, ikey<RANK> src) {
    for (rank_t i = 0; i < ds_num; i++) {
      ds_modifier::move(m_data_sets[i], dst, src);
    }
  }

  template<rank_t RANK>
  void swap(ikey<RANK> a, ikey<RANK> b) {
    for (rank_t i = 0; i < ds_num; i++) {
      ds_modifier::swap(m_data_sets[i], a, b);
    }
  }

  template<rank_t RANK>
  void null(ikey<RANK> key) {
    for (rank_t i = 0; i < ds_num; i++) {
      ds_modifier::null(m_data_sets[i], key);
    }
  }

  void clear_null() {
    for (rank_t i = 0; i < ds_num; i++) {
      ds_modifier::clear_null(m_data_sets[i]);
    }
  }

  template<rank_t RANK>
  void merge(ikey<RANK> dst, ikey<RANK> src) {
    for (rank_t i = 0; i < ds_num; i++) {
      ds_modifier::merge(m_data_sets[i], dst, src);
    }
  }

  std::array<DS, DS_NUM> m_data_sets;
};
} // namespace dlm
