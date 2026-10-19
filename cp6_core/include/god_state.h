#pragma once
#include "tuple_utils.h"
#include <tuple>
#include <type_traits>
#include <utility>

template<typename T>
concept has_template_t =
  requires(T t) { typename T::template template_t<unsigned>; };

template<typename T, typename TYPE_WRAPPER>
using full_t = decltype([] {
  if constexpr (has_template_t<T>) {
    return typename T::template template_t<TYPE_WRAPPER>{};
  } else {
    return T{};
  }
}());

template<typename TYPE_WRAPPER>
constexpr size_t wrapper_size_v =
  std::tuple_size_v<typename TYPE_WRAPPER::types>;

template<std::size_t I, typename TYPE_WRAPPER>
using wrapper_element_t =
  std::tuple_element_t<I, typename TYPE_WRAPPER::types>;

template<std::size_t I, typename TYPE_WRAPPER>
using wrapper_element_ft =
  full_t<wrapper_element_t<I, TYPE_WRAPPER>, TYPE_WRAPPER>;

template<typename TYPE_WRAPPER>
using full_tuple_t =
  decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple<wrapper_element_ft<Is, TYPE_WRAPPER>...>{};
  }(std::make_index_sequence<wrapper_size_v<TYPE_WRAPPER>>{}));

namespace god {
template<typename TYPE_WRAPPER>
struct state {
  state() {
    [&]<std::size_t... Is>(std::index_sequence<Is...>) {
      (
        [&] {
          if constexpr (
            requires { get<Is>(m_states).init(*this); } ||
            requires {
              std::tuple_element_t<
                Is,
                full_tuple_t<TYPE_WRAPPER>
              >::god::state_init;
            }
          ) {
            get<Is>(m_states).init(*this);
          }
        }(),
        ...);
    }(std::make_index_sequence<wrapper_size_v<TYPE_WRAPPER>>{});
  }

  full_tuple_t<TYPE_WRAPPER> m_states;
};
} // namespace god

template<std::size_t Is, typename TW>
inline decltype(auto) get(god::state<TW> &state) {
  static_assert(Is < wrapper_size_v<TW>);
  return std::get<Is>(state.m_states);
}

template<std::size_t Is, typename TW>
inline decltype(auto) get(const god::state<TW> &state) {
  static_assert(Is < wrapper_size_v<TW>);
  return std::get<Is>(state.m_states);
}

template<typename T, typename TW>
inline decltype(auto) get(god::state<TW> &state) {
  return std::get<full_t<T, TW>>(state.m_states);
}

template<typename T, typename TW>
inline decltype(auto) get(const god::state<TW> &state) {
  return std::get<full_t<T, TW>>(state.m_states);
}

template<template<typename> class COND, typename TW>
inline decltype(auto) get(god::state<TW> &state) {
  static_assert(
    std::tuple_size_v<
      std::decay_t<decltype(tuple_ref_if<COND>(state.m_states))>
    > == 1,
    "there must be exaclty one element in the tuple that satisfies "
    "COND"
  );
  return std::get<0>(tuple_ref_if<COND>(state.m_states));
}

template<template<typename> class COND, typename TW>
inline decltype(auto) get(const god::state<TW> &state) {
  static_assert(
    std::tuple_size_v<
      std::decay_t<decltype(tuple_ref_if<COND>(state.m_states))>
    > == 1,
    "there must be exaclty one element in the tuple that satisfies "
    "COND"
  );
  return std::get<0>(tuple_ref_if<COND>(state.m_states));
}

template<template<typename> class PRED, typename TW>
inline auto get_matching(god::state<TW> &state) {
  return tuple_ref_if<PRED>(state.m_states);
}

template<template<typename> class PRED, typename TW>
inline auto get_matching(const god::state<TW> &state) {
  return tuple_ref_if<PRED>(state.m_states);
}
