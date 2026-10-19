#pragma once
#include "ckey.h"

static constexpr rank_t N = (rank_t) - 1;
// a compile time indirection on the concatenation of k1 and k2
template<rank_t... IND, rank_t R1, rank_t R2>
auto merge_keys(const dlm::ikey<R1> &k1, const dlm::ikey<R2> &k2, dlm::ckey<IND...> c = {}) {
  static_assert(R1 + R2 == sizeof...(IND));
  static constexpr rank_t ind[] = {IND...};
  static constexpr rank_t num = sizeof...(IND);
  static constexpr rank_t valid_num = [] {
    rank_t num = 0;
    for (unsigned i = 0; i < sizeof...(IND); i++) {
      if (ind[i] != N) {
        num++;
      }
    }
    return num;
  }();

  // compile time checks
  {
    static constexpr bool in_bounds = [] {
      for (rank_t i = 0; i < num; i++) {
        if (!(ind[i] < valid_num) && ind[i] != N) {
          return false;
        }
      }
      return true;
    }();

    static_assert(in_bounds, "Indirection out of bounds");
    if constexpr (in_bounds) {
      static constexpr bool is_dense = [] {
        bool proj[valid_num]{};
        for (rank_t i = 0; i < num; i++) {
          if (ind[i] != N) proj[ind[i]] = true;
        }
        for (rank_t i = 0; i < valid_num; i++) {
          if (proj[i] != true) {
            return false;
          }
        }
        return true;
      }();

      static_assert(
        is_dense,
        "Non null indirection must take every input and map to distinct outputs"
      );
    }
  }

  dlm::ikey<valid_num> end_key;
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (
      [&] {
        if constexpr (ind[Is] != N) end_key[ind[Is]] = k1[Is];
      }(),
      ...
    );
  }(std::make_index_sequence<R1>{});

  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (
      [&] {
        if constexpr (ind[R1 + Is] != N) end_key[ind[R1 + Is]] = k2[Is];
      }(),
      ...
    );
  }(std::make_index_sequence<R2>{});

  return end_key;
}

template<rank_t N>
struct rank_arr {
  rank_t arr[N];
};

template<rank_t R1, rank_t R2, rank_t... IND, rank_t MR>
auto unmerge_key1(const dlm::ikey<MR> &merged_key, dlm::ckey<IND...> c = {}) {
  static_assert(R1 + R2 == sizeof...(IND));
  static constexpr rank_t ind[] = {IND...};
  static constexpr rank_t num = sizeof...(IND);
  static constexpr rank_t valid_num = [] {
    rank_t num = 0;
    for (unsigned i = 0; i < sizeof...(IND); i++) {
      if (ind[i] != N) {
        num++;
      }
    }
    return num;
  }();
  static_assert(MR == valid_num, "Merged key is not valid");

  static constexpr rank_arr<valid_num> inv_ind = [] {
    rank_arr<valid_num> inv_ind;
    for (unsigned i = 0; i < num; i++) {
      if (ind[i] != N) {
        inv_ind.arr[ind[i]] = i;
      }
    }
    return inv_ind;
  }();

  // compile time checks
  {
    static constexpr bool in_bounds = [] {
      for (rank_t i = 0; i < num; i++) {
        if (!(ind[i] < valid_num) && ind[i] != N) {
          return false;
        }
      }
      return true;
    }();

    static_assert(in_bounds, "Indirection out of bounds");
    if constexpr (in_bounds) {
      static constexpr bool is_dense = [] {
        bool proj[valid_num]{};
        for (rank_t i = 0; i < num; i++) {
          if (ind[i] != N) proj[ind[i]] = true;
        }
        for (rank_t i = 0; i < valid_num; i++) {
          if (proj[i] != true) {
            return false;
          }
        }
        return true;
      }();

      static_assert(
        is_dense,
        "Non null indirection must take every input and map to distinct outputs"
      );
    }
  }

  dlm::ikey<R1> end_key;
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (
      [&] {
        if constexpr (inv_ind.arr[Is] < R1) end_key[inv_ind.arr[Is]] = merged_key[Is];
      }(),
      ...
    );
  }(std::make_index_sequence<valid_num>{});
  return end_key;
}

template<rank_t R1, rank_t R2, rank_t... IND, rank_t MR>
auto unmerge_key2(const dlm::ikey<MR> &merged_key, dlm::ckey<IND...> c = {}) {
  static constexpr rank_t ind[] = {IND...};
  static constexpr rank_t num = sizeof...(IND);
  static constexpr rank_t valid_num = [] {
    rank_t num = 0;
    for (unsigned i = 0; i < sizeof...(IND); i++) {
      if (ind[i] != N) {
        num++;
      }
    }
    return num;
  }();
  static_assert(MR == valid_num, "Merged key is not valid");

  static constexpr auto inv_ind = [] {
    rank_arr<valid_num> inv_ind;
    for (unsigned i = 0; i < num; i++) {
      if (ind[i] != N) {
        inv_ind.arr[ind[i]] = i;
      }
    }
    return inv_ind;
  }();

  // compile time checks
  {
    static constexpr bool in_bounds = [] {
      for (rank_t i = 0; i < num; i++) {
        if (!(ind[i] < valid_num) && ind[i] != N) {
          return false;
        }
      }
      return true;
    }();

    static_assert(in_bounds, "Indirection out of bounds");
    if constexpr (in_bounds) {
      static constexpr bool is_dense = [] {
        bool proj[valid_num]{};
        for (rank_t i = 0; i < num; i++) {
          if (ind[i] != N) proj[ind[i]] = true;
        }
        for (rank_t i = 0; i < valid_num; i++) {
          if (proj[i] != true) {
            return false;
          }
        }
        return true;
      }();

      static_assert(
        is_dense,
        "Non null indirection must take every input and map to distinct outputs"
      );
    }
  }

  dlm::ikey<R2> end_key;
  [&]<size_t... Is>(std::index_sequence<Is...>) {
    (
      [&] {
        if constexpr (inv_ind.arr[Is] >= R1) end_key[inv_ind.arr[Is] - R1] = merged_key[Is];
      }(),
      ...
    );
  }(std::make_index_sequence<valid_num>{});
  return end_key;
}
