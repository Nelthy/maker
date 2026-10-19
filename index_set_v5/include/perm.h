#pragma once
#include "data_set_tuple.h"
#include "operation_ids.h"
#include <type_traits>

namespace dlm {
template<typename DATA_SET, typename... DATA_SETS>
class perm {
public:
  static constexpr rank_t rank = DATA_SET::rank - 1;
  static_assert(
    ((DATA_SETS::rank == rank + 1) && ...),
    "each data set must increase the set's rank by one"
  );

  template<rank_t KEY_DEPTH = 0>
  unsigned size(ikey<KEY_DEPTH> key = {}) const {
    return ds_modifier::size(m_data_sets, key);
  }

  template<std::size_t I>
  auto &get() {
    return m_data_sets.template get<I>();
  }

  template<std::size_t I>
  const auto &get() const {
    return m_data_sets.template get<I>();
  }

  std::conditional_t<rank == 0, unsigned, std::false_type> operation_id{};

protected:
  friend class ds_modifier;

  template<rank_t KEY_DEPTH = 0>
  void resize(index_t new_size, ikey<KEY_DEPTH> key = {}) {
    ds_modifier::resize(m_data_sets, new_size, key);
  }

  template<rank_t KEY_DEPTH>
  void move(ikey<KEY_DEPTH> dst, ikey<KEY_DEPTH> src) {
    ds_modifier::move(m_data_sets, dst, src);
  }

  template<rank_t KEY_DEPTH>
  void swap(ikey<KEY_DEPTH> a, ikey<KEY_DEPTH> b) {
    ds_modifier::swap(m_data_sets, a, b);
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    ds_modifier::null(m_data_sets, key);
  }

  void clear_null() { ds_modifier::clear_null(m_data_sets); }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    ds_modifier::clear(m_data_sets, key);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    ds_modifier::merge(m_data_sets, dst, src);
  }

  dst<DATA_SET, DATA_SETS...> m_data_sets;
};

} // namespace dlm
