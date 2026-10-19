#pragma once
#include "data_set_tuple.h"
#include "ds_utils.h"

#define DEFINE_TUPLE_DS_FIELD(post_tuple_perm, name, index_name)                                   \
  template<unsigned index_name>                                                                    \
  static constexpr auto name##_n() {                                                               \
    return dlm::ckey<index_name, UNPAREN(post_tuple_perm)>{};                                      \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  auto &name() {                                                                                   \
    return dlm::get<index_name, UNPAREN(post_tuple_perm)>(*this);                                  \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  const auto &name() const {                                                                       \
    return dlm::get<index_name, UNPAREN(post_tuple_perm)>(*this);                                  \
  }

#define DEFINE_TUPLE_DS_FIELD_PRE(pre_tuple_perm, post_tuple_perm, name, index_name)               \
  template<unsigned index_name>                                                                    \
  static constexpr auto name##_n() {                                                               \
    return dlm::ckey<UNPAREN(pre_tuple_perm), index_name, UNPAREN(post_tuple_perm)>{};             \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  auto &name() {                                                                                   \
    return dlm::get<UNPAREN(pre_tuple_perm), index_name, UNPAREN(post_tuple_perm)>(*this);         \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  const auto &name() const {                                                                       \
    return dlm::get<UNPAREN(pre_tuple_perm), index_name, UNPAREN(post_tuple_perm)>(*this);         \
  }

namespace dlm {
template<index_t DS_NUM, template<index_t> class GEN_T>
class perm_tuple {
public:
  static constexpr rank_t rank = GEN_T<0>::type::rank - 1;
  static_assert(
    []<size_t... Ps>(std::index_sequence<Ps...>) {
      return ((GEN_T<Ps>::type::rank == rank + 1) && ...);
    }(std::make_index_sequence<DS_NUM>{}),
    "each permute must increase the set's rank by one"
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

  template<rank_t KEY_DEPTH = 0>
  void clear(ikey<KEY_DEPTH> key = {}) {
    ds_modifier::clear(m_data_sets, key);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    ds_modifier::merge(m_data_sets, dst, src);
    // ds_modifier::merge_impl(m_data_sets, dst, src);
  }

private:
  decltype([]<size_t... Ps>(std::index_sequence<Ps...>) {
    return dst<typename GEN_T<Ps>::type...>{};
  }(std::make_index_sequence<DS_NUM>{})) m_data_sets;
};
} // namespace dlm
