#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include <tuple>
#include <utility>

namespace dlm {
template<typename DS, typename... DSs>
class flat_data_set_tuple: public DS, public DSs... {
  static_assert(((DS::rank == DSs::rank) && ...), "all data sets in tuple must have the same rank");

public:
  static constexpr rank_t rank = DS::rank;
  static constexpr rank_t ds_num = sizeof...(DSs) + 1;

  template<rank_t I>
  auto &get() {
    using child_t = std::tuple_element_t<I, std::tuple<DS, DSs...>>;
    return static_cast<child_t &>(*this);
  }

  template<rank_t I>
  const auto &get() const {
    using child_t = std::tuple_element_t<I, std::tuple<DS, DSs...>>;
    return static_cast<const child_t &>(*this);
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
      (ds_modifier::resize(get<Is + 1>(), new_size, key), ...);
    }(std::make_index_sequence<sizeof...(DSs)>{});
    // size ds must be updated last
    ds_modifier::resize(get<0>(), new_size, key);
  }

  template<rank_t RANK>
  void move(ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::move(get<Is>(), dst, src), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void merge(ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::merge(get<Is>(), dst, src), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void move(flat_data_set_tuple &other, ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::move(get<Is>(), (other.template get<Is>()), dst, src), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void swap(ikey<RANK> a, ikey<RANK> b) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::swap(get<Is>(), a, b), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void swap(flat_data_set_tuple &other, ikey<RANK> a, ikey<RANK> b) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::swap(get<Is>(), other.template get<Is>(), a, b), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void null(ikey<RANK> key) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::null(get<Is>(), key), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  void clear_null() {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::clear_null(get<Is>()), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void clear(ikey<RANK> key) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::clear(get<Is>(), key), ...);
    }(std::make_index_sequence<ds_num>{});
  }

  template<rank_t RANK>
  void copy(const flat_data_set_tuple &other, ikey<RANK> dst, ikey<RANK> src) {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (ds_modifier::copy(get<Is>(), other.template get<Is>(), dst, src), ...);
    }(std::make_index_sequence<sizeof...(DSs) + 1>{});
  }
};

template<typename... DSs>
using fdst = flat_data_set_tuple<DSs...>;
} // namespace dlm
