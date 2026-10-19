#pragma once
#include <array>
#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

template<typename FUNC, typename Tuple>
void tuple_for_each(FUNC &&f, Tuple &&t) {
  [f = std::forward<FUNC>(f), &t]<std::size_t... Is>(std::index_sequence<Is...>) {
    (f(std::get<Is>(std::forward<Tuple>(t))), ...);
  }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

template<typename FUNC, typename Tuple>
void tuple_for_each_i(FUNC &&f, Tuple &&t) {
  [f = std::forward<FUNC>(f), &t]<std::size_t... Is>(std::index_sequence<Is...>) {
    (f.template operator()<Is>(std::get<Is>(std::forward<Tuple>(t))), ...);
  }(std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}

template<std::size_t Np, typename FUNC>
void for_each_i(FUNC &&f) {
  [f = std::forward<FUNC>(f)]<std::size_t... Is>(std::index_sequence<Is...>) {
    (f.template operator()<Is>(), ...);
  }(std::make_index_sequence<Np>{});
};

// function to redirect the main thread to do work instead of spinning. Must be defined by user.
void on_main_spin();

// a loop that only returns once all function templates returns true.
// Only calls each function once
template<unsigned NUM, typename FUNC>
void spin_loop_i(FUNC &&f) {
  bool is_done[NUM]{};
  unsigned done_num = 0;
  while (done_num < NUM) {
    for_each_i<NUM>([&]<size_t I> {
      if (!is_done[I]) {
        if (f.template operator()<I>()) {
          is_done[I] = true;
          done_num++;
        };
      }
    });
    on_main_spin();
  }
};

// a loop that only returns once all functions returns true.
// inputs a tuple element into each function.
// Only calls each function once
template<typename FUNC, typename TUPLE>
void tuple_spin_loop(FUNC &&f, TUPLE &t) {
  static constexpr size_t num = std::tuple_size_v<TUPLE>;
  bool is_done[num]{};
  unsigned done_num = 0;
  while (done_num < num) {
    for_each_i<num>([&]<size_t I> {
      if (!is_done[I]) {
        if (f(std::get<I>(t))) {
          is_done[I] = true;
          done_num++;
        };
      }
    });
    on_main_spin();
  }
};

// returns an indirection from the subset where COND is true to the full set.
template<template<typename> typename COND, typename... Ts>
constexpr auto filter_pack() {
  constexpr bool matches[] = {COND<Ts>::value...};
  constexpr std::size_t matching_num = ((COND<Ts>::value ? 1 : 0) + ...);
  std::array<std::size_t, matching_num> result;
  std::size_t fi = 0;
  for (std::size_t ui = 0; ui < sizeof...(Ts); ui++)
    if (matches[ui]) result[fi++] = ui;

  return result;
}

// returns an indirection from the subset where COND is true to the full set.
template<template<typename> typename COND, typename TUPLE>
constexpr auto filter_tuple() {
  return []<std::size_t... Is>(std::index_sequence<Is...>) {
    return filter_pack<COND, std::tuple_element_t<Is, TUPLE>...>();
  }(std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

template<template<typename> typename COND, typename TUPLE>
auto tuple_ref_if(TUPLE &t) {
  // filtered index to unfiltered index
  constexpr auto fi_to_ui = filter_tuple<COND, TUPLE>();

  return [&]<std::size_t... Fs>(std::index_sequence<Fs...>) {
    return std::tuple<std::tuple_element_t<fi_to_ui[Fs], TUPLE> &...>{
      (std::get<fi_to_ui[Fs]>(t))...
    };
  }(std::make_index_sequence<fi_to_ui.size()>{});
}

template<template<typename> typename COND, typename TUPLE>
auto tuple_ref_if(const TUPLE &t) {
  // filtered index to unfiltered index
  constexpr auto fi_to_ui = filter_tuple<COND, TUPLE>();

  return [&]<std::size_t... Fs>(std::index_sequence<Fs...>) {
    return std::tuple<std::tuple_element_t<fi_to_ui[Fs], TUPLE> &...>{
      (std::get<fi_to_ui[Fs]>(t))...
    };
  }(std::make_index_sequence<fi_to_ui.size()>{});
}

// returns the index of the first type that is the same as T. Returns -1 if no match
template<typename T, typename TUPLE>
constexpr unsigned first_same() {
  return []<std::size_t... Is>(std::index_sequence<Is...>) {
    constexpr bool compares[] = {std::is_same<T, std::tuple_element_t<Is, TUPLE>>::value...};
    for (unsigned i = 0; i < std::tuple_size_v<TUPLE>; i++) {
      if (compares[i]) return i;
    }
    return static_cast<unsigned>(-1);
  }(std::make_index_sequence<std::tuple_size_v<TUPLE>>{});
}

template<typename FROM, typename TO>
constexpr auto generate_tuple_indirection() {
  return []<std::size_t... Is>(std::index_sequence<Is...>) {
    return std::array<unsigned, std::tuple_size_v<FROM>>{
      first_same<std::tuple_element_t<Is, FROM>, TO>()...
    };
  }(std::make_index_sequence<std::tuple_size_v<FROM>>{});
}

template<template<typename> class COND, typename TUPLE>
static constexpr unsigned get_matching_num() {
  return filter_tuple<COND, TUPLE>().size();
}

template<unsigned SIZE, template<unsigned> class GEN>
using generated_tuple = decltype([]<std::size_t... Is>(std::index_sequence<Is...>) {
  return std::tuple<typename GEN<Is>::type...>{};
}(std::make_index_sequence<SIZE>{}));

template<typename T, typename... Us>
inline constexpr std::size_t type_count_v = ((std::is_same_v<T, Us> ? 1 : 0) + ...);

template<typename... Ts>
struct all_types_unique: std::bool_constant<((type_count_v<Ts, Ts...> == 1) && ...)> {};

template<typename... Ts>
inline constexpr bool all_types_unique_v = all_types_unique<Ts...>::value;

template<typename TUPLE>
struct tuple_types_unique;

template<typename... Ts>
struct tuple_types_unique<std::tuple<Ts...>>: all_types_unique<Ts...> {};

template<typename TUPLE>
inline constexpr bool tuple_types_unique_v = tuple_types_unique<TUPLE>::value;

template<typename FIRST_T>
struct same_as {
  template<typename T>
  using cond = std::is_same<FIRST_T, T>;
};

template<template<typename> class COND, typename TUPLE>
using get_matching_ts = decltype([] {
  static constexpr auto filtered = filter_tuple<COND, TUPLE>();
  return []<std::size_t... Is>(std::index_sequence<Is...>) {
    static constexpr auto _filtered = filter_tuple<COND, TUPLE>();
    return std::tuple<std::tuple_element_t<_filtered[Is], TUPLE>...>{};
  }(std::make_index_sequence<filtered.size()>{});
}());

template<template<typename> class COND, typename TUPLE>
using get_t = decltype([] {
  static constexpr auto filtered = filter_tuple<COND, TUPLE>();
  static_assert(filtered.size() == 1, "tuple must have exactly one type that satisfies COND");
  return std::tuple_element_t<filtered[0], TUPLE>{};
}());
