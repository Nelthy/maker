#pragma once
#include "basic_types.h"
#include "data_set_tuple.h"
#include "ds_modifier.h"
#include "ikey.h"
#include <array>
#include <tuple>

namespace dlm {

#define DEFINE_ARRAY_DS_FIELD(post_array_perm, name, index_name)                                   \
  template<unsigned index_name>                                                                    \
  static constexpr auto name##_n = dlm::ckey<index_name, UNPAREN(post_array_perm)>{};              \
  template<unsigned index_name>                                                                    \
  auto &name() {                                                                                   \
    return dlm::get<index_name, UNPAREN(post_array_perm)>(*this);                                  \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  const auto &name() const {                                                                       \
    return dlm::get<index_name, UNPAREN(post_array_perm)>(*this);                                  \
  }                                                                                                \
  auto &name##_rt(unsigned index_name) {                                                           \
    return dlm::get<UNPAREN(post_array_perm)>(this->get(index_name));                              \
  }                                                                                                \
  const auto &name##_rt(unsigned index_name) const {                                               \
    return dlm::get<UNPAREN(post_array_perm)>(this->get(index_name));                              \
  }

#define DEFINE_ARRAY_DS_FIELD_PRE(pre_array_perm, post_array_perm, name, index_name)               \
  template<unsigned index_name>                                                                    \
  static constexpr auto name##_n =                                                                 \
    dlm::ckey<UNPAREN(pre_array_perm), index_name, UNPAREN(post_array_perm)>{};                    \
                                                                                                   \
  template<unsigned index_name>                                                                    \
  auto &name() {                                                                                   \
    return dlm::get<UNPAREN(pre_array_perm), index_name, UNPAREN(post_array_perm)>(*this);         \
  }                                                                                                \
  template<unsigned index_name>                                                                    \
  const auto &name() const {                                                                       \
    return dlm::get<UNPAREN(pre_array_perm), index_name, UNPAREN(post_array_perm)>(*this);         \
  }                                                                                                \
  auto &name##_rt(unsigned index_name) {                                                           \
    return dlm::get<UNPAREN(post_array_perm)>(                                                     \
      dlm::get<UNPAREN(pre_array_perm)>().get(index_name)                                          \
    );                                                                                             \
  }                                                                                                \
  const auto &name##_rt(unsigned index_name) const {                                               \
    return dlm::get<UNPAREN(post_array_perm)>(                                                     \
      dlm::get<UNPAREN(pre_array_perm)>().get(index_name)                                          \
    );                                                                                             \
  }

template<rank_t DS_NUM, typename DATA_SET, typename... EXTRA_DSs>
class perm_arr {
public:
  static constexpr rank_t rank = DATA_SET::rank - 1;
  static_assert(DATA_SET::rank == rank + 1);

  template<rank_t KEY_DEPTH = 0>
  unsigned size(ikey<KEY_DEPTH> key) const {
    return ds_modifier::size(std::get<0>(m_data_sets), key);
  }

  template<std::size_t I>
  auto &get() {
    if constexpr (I < DS_NUM) {
      return std::get<I>(m_data_sets);
    } else {
      return m_extra_ds.template get<DS_NUM - I>();
    }
  }

  template<std::size_t I>
  const auto &get() const {
    if constexpr (I < DS_NUM) {
      return std::get<I>(m_data_sets);
    } else {
      return m_extra_ds.template get<DS_NUM - I>();
    }
  }

  auto &get(index_t i) { return m_data_sets[i]; }

  const auto &get(index_t i) const { return m_data_sets[i]; }

  std::conditional_t<rank == 0, unsigned, std::false_type> operation_id{};

protected:
  friend class ds_modifier;

  template<rank_t KEY_DEPTH = 0>
  void resize(index_t new_size, ikey<KEY_DEPTH> key = {}) {
    std::apply(
      [&](auto &&...perm) { (ds_modifier::resize(perm, new_size, key), ...); },
      m_data_sets
    );
    ds_modifier::resize(m_extra_ds, new_size, key);
  }

  template<rank_t KEY_DEPTH>
  void move(ikey<KEY_DEPTH> dst, ikey<KEY_DEPTH> src) {
    std::apply([&](auto &&...perm) { (ds_modifier::move(perm, dst, src), ...); }, m_data_sets);
    ds_modifier::move(m_extra_ds, dst, src);
  }

  template<rank_t KEY_DEPTH>
  void swap(ikey<KEY_DEPTH> a, ikey<KEY_DEPTH> b) {
    std::apply([&](auto &&...perm) { (ds_modifier::swap(perm, a, b), ...); }, m_data_sets);
    ds_modifier::swap(m_extra_ds, a, b);
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    std::apply([&key](auto &&...perm) { (ds_modifier::null(perm, key), ...); }, m_data_sets);
    ds_modifier::null(m_extra_ds, key);
  }

  void clear_null() {
    std::apply([](auto &&...perm) { (ds_modifier::clear_null(perm), ...); }, m_data_sets);
    ds_modifier::clear_null(m_extra_ds);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    std::apply([&](auto &&...perm) { (ds_modifier::merge(perm, dst, src), ...); }, m_data_sets);
    ds_modifier::merge(m_extra_ds, dst, src);
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    std::apply([&](auto &&...perm) { (ds_modifier::clear(perm, key), ...); }, m_data_sets);
    ds_modifier::clear(m_extra_ds, key);
  }

private:
  std::array<DATA_SET, DS_NUM> m_data_sets;
  dst<EXTRA_DSs...> m_extra_ds;
};

template<rank_t DS_NUM, typename DATA_SET>
class perm_arr<DS_NUM, DATA_SET> {
public:
  static constexpr rank_t rank = DATA_SET::rank - 1;
  static_assert(DATA_SET::rank == rank + 1);

  template<rank_t KEY_DEPTH = 0>
  unsigned size(ikey<KEY_DEPTH> key) const {
    return ds_modifier::size(std::get<0>(m_data_sets), key);
  }

  template<std::size_t I>
  auto &get() {
    return std::get<I>(m_data_sets);
  }

  template<std::size_t I>
  const auto &get() const {
    return std::get<I>(m_data_sets);
  }

  auto &get(index_t i) { return m_data_sets[i]; }

  const auto &get(index_t i) const { return m_data_sets[i]; }

  std::conditional_t<rank == 0, unsigned, std::false_type> operation_id{};

protected:
  friend class ds_modifier;

  template<rank_t KEY_DEPTH = 0>
  void resize(index_t new_size, ikey<KEY_DEPTH> key = {}) {
    std::apply(
      [&](auto &&...perm) { (ds_modifier::resize(perm, new_size, key), ...); },
      m_data_sets
    );
  }

  template<rank_t KEY_DEPTH>
  void move(ikey<KEY_DEPTH> dst, ikey<KEY_DEPTH> src) {
    std::apply([&](auto &&...perm) { (ds_modifier::move(perm, dst, src), ...); }, m_data_sets);
  }

  template<rank_t KEY_DEPTH>
  void swap(ikey<KEY_DEPTH> a, ikey<KEY_DEPTH> b) {
    std::apply([&](auto &&...perm) { (ds_modifier::swap(perm, a, b), ...); }, m_data_sets);
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    std::apply([&key](auto &&...perm) { (ds_modifier::null(perm, key), ...); }, m_data_sets);
  }

  void clear_null() {
    std::apply([](auto &&...perm) { (ds_modifier::clear_null(perm), ...); }, m_data_sets);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    std::apply([&](auto &&...perm) { (ds_modifier::merge(perm, dst, src), ...); }, m_data_sets);
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    std::apply([&](auto &&...perm) { (ds_modifier::clear(perm, key), ...); }, m_data_sets);
  }

private:
  std::array<DATA_SET, DS_NUM> m_data_sets;
};
} // namespace dlm
