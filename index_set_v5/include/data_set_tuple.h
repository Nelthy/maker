#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include <tuple>
#include <utility>

namespace dlm {
template<typename DS, typename... DSs>
class data_set_tuple {
  static_assert(((DS::rank == DSs::rank) && ...), "all data sets in tuple must have the same rank");

public:
  static constexpr rank_t rank = DS::rank;
  static constexpr rank_t ds_num = sizeof...(DSs) + 1;

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
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::resize(std::get<Is + 1>(m_data_sets), new_size, key), ...);
    }(std::make_index_sequence<sizeof...(DSs)>{});
    // size ds must be updated last
    ds_modifier::resize(std::get<0>(m_data_sets), new_size, key);
  }

  template<rank_t RANK>
  void move(ikey<RANK> dst, ikey<RANK> src) {
    std::apply(
      [&](auto &&...data_sets) { (ds_modifier::move(data_sets, dst, src), ...); },
      m_data_sets
    );
  }

  template<rank_t RANK>
  void merge(ikey<RANK> dst, ikey<RANK> src) {
    std::apply(
      [&](auto &&...data_sets) { (ds_modifier::merge(data_sets, dst, src), ...); },
      m_data_sets
    );
  }

  template<rank_t RANK>
  void move(data_set_tuple &other, ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::move(std::get<Is>(m_data_sets), std::get<Is>(other.m_data_sets), dst, src),
       ...);
    }(std::make_index_sequence<sizeof...(DSs) + 1>{});
  }

  template<rank_t RANK>
  void swap(ikey<RANK> a, ikey<RANK> b) {
    std::apply(
      [&](auto &&...data_sets) { (ds_modifier::swap(data_sets, a, b), ...); },
      m_data_sets
    );
  }

  template<rank_t RANK>
  void swap(data_set_tuple &other, ikey<RANK> a, ikey<RANK> b) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::swap(std::get<Is>(m_data_sets), std::get<Is>(other.m_data_sets), a, b), ...);
    }(std::make_index_sequence<sizeof...(DSs) + 1>{});
  }

  template<rank_t RANK>
  void null(ikey<RANK> key) {
    std::apply([&](auto &&...data_sets) { (ds_modifier::null(data_sets, key), ...); }, m_data_sets);
  }

  void clear_null() {
    std::apply(
      [&](auto &&...data_sets) { (ds_modifier::clear_null(data_sets), ...); },
      m_data_sets
    );
  }

  template<rank_t RANK>
  void clear(ikey<RANK> key) {
    std::apply([&](auto &&...data_sets) { (ds_modifier::clear(data_sets, key), ...); }, m_data_sets);
  }

  template<rank_t RANK>
  void copy(const data_set_tuple &other, ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::copy(std::get<Is>(m_data_sets), std::get<Is>(other.m_data_sets), dst, src),
       ...);
    }(std::make_index_sequence<sizeof...(DSs) + 1>{});
  }

  std::tuple<DS, DSs...> m_data_sets;
};

template<typename... DSs>
using dst = data_set_tuple<DSs...>;
} // namespace dlm
