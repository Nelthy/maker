#pragma once
#include "ds_modifier.h"
#include "ikey.h"
#include "tri_tuple.h"
#include "tri_utils.h"
#include <tuple>
#include <utility>

#define DEFINE_TRI_DS_FIELD(post_tri_perm, name, tri_index_name, tri_dim)                          \
  template<unsigned tri_index_name##1, unsigned tri_index_name##2>                                 \
  static constexpr auto name##_n() {                                                                 \
    return dlm::ckey<                                                                              \
      tri_linearize(tri_index_name##1, tri_index_name##2, tri_dim),                                \
      UNPAREN(post_tri_perm)                                                                       \
    >{};                                                                                           \
  }                                                                                                \
  template<unsigned tri_index_name##1, unsigned tri_index_name##2>                                 \
  inline auto &name() {                                                                            \
    return dlm::get<                                                                               \
      tri_linearize(tri_index_name##1, tri_index_name##2, tri_dim),                                \
      UNPAREN(post_tri_perm)                                                                       \
    >(*this);                                                                                      \
  }                                                                                                \
  template<unsigned tri_index_name##1, unsigned tri_index_name##2>                                 \
  inline const auto &name() const {                                                                \
    return dlm::get<                                                                               \
      tri_linearize(tri_index_name##1, tri_index_name##2, tri_dim),                                \
      UNPAREN(post_tri_perm)                                                                       \
    >(*this);                                                                                      \
  }


namespace dlm {
template<index_t DIM, template<index_t, index_t> class GEN_T>
class perm_tri_tuple {
public:
  static constexpr index_t perm_num = tri_linearize(DIM - 1, DIM - 1, DIM) + 1;
  static constexpr rank_t rank = GEN_T<0, 0>::type::rank - 1;

  static_assert([]<size_t... Ps>(std::index_sequence<Ps...>) {
    return (
      (std::decay_t<decltype(std::get<Ps>(std::declval<tri_tuple<DIM, GEN_T>>().m_tuple))>::rank ==
       rank + 1) &&
      ...
    );
  }(std::make_index_sequence<perm_num>{}));

  template<rank_t KEY_DEPTH = 0>
  unsigned size(ikey<KEY_DEPTH> key = {}) const {
    return ds_modifier::size(std::get<0>(m_permutes.m_tuple), key);
  }

  template<rank_t P>
  auto &get() {
    return std::get<P>(m_permutes.m_tuple);
  }

  template<rank_t P>
  const auto &get() const {
    return std::get<P>(m_permutes.m_tuple);
  }

  template<unsigned A, unsigned B, unsigned DS>
  auto &tri_get(dlm::ckey<A, B, DS> c = {}) {
    return get<tri_linearize(A, B, DIM)>().template get<DS>();
  }

  template<unsigned A, unsigned B, unsigned DS>
  const auto &tri_get(dlm::ckey<A, B, DS> c = {}) const {
    return get<tri_linearize(A, B, DIM)>().template get<DS>();
  }

  std::conditional_t<rank == 0, unsigned, std::false_type> operation_id{};

protected:
  friend class ds_modifier;

  template<rank_t KEY_DEPTH>
  void resize(index_t new_size, ikey<KEY_DEPTH> key = {}) {
    std::apply(
      [&](auto &&...perm) { (ds_modifier::resize(perm, new_size, key), ...); },
      m_permutes.m_tuple
    );
  }

  template<rank_t KEY_DEPTH>
  void move(ikey<KEY_DEPTH> dst, ikey<KEY_DEPTH> src) {
    std::apply(
      [&](auto &&...perm) { (ds_modifier::move(perm, dst, src), ...); },
      m_permutes.m_tuple
    );
  }

  template<rank_t KEY_DEPTH>
  void swap(ikey<KEY_DEPTH> a, ikey<KEY_DEPTH> b) {
    std::apply([&](auto &&...perm) { (ds_modifier::swap(perm, a, b), ...); }, m_permutes.m_tuple);
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    std::apply([&key](auto &&...perm) { (ds_modifier::null(perm, key), ...); }, m_permutes.m_tuple);
  }

  void clear_null() {
    std::apply([](auto &&...perm) { (ds_modifier::clear_null(perm), ...); }, m_permutes.m_tuple);
  }

  template<rank_t KEY_DEPTH = 0>
  void clear(ikey<KEY_DEPTH> key) {
    std::apply([&](auto &&...perm) { (ds_modifier::clear(perm, key), ...); }, m_permutes.m_tuple);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    std::apply(
      [&](auto &&...perm) { (ds_modifier::merge(perm, dst, src), ...); },
      m_permutes.m_tuple
    );
  }

  tri_tuple<DIM, GEN_T> m_permutes;
};
} // namespace dlm
