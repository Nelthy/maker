#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include "get.h"
#include <cassert>
#include <type_traits>
#include <utility>

#define UNPAREN(...) UNPAREN_IMPL __VA_ARGS__
#define UNPAREN_IMPL(...) __VA_ARGS__

#define DEFINE_DS_FIELD(perm, name)                                                                \
  static constexpr dlm::ckey<UNPAREN(perm)> name##_n = {};                                         \
  inline auto &name() { return dlm::get<UNPAREN(perm)>(*this); }                                   \
  inline const auto &name() const { return dlm::get<UNPAREN(perm)>(*this); }

namespace dlm {

template<typename T, rank_t CURR_RANK = 0>
constexpr rank_t traversable_rank() {
  if constexpr (has_bracket<T>) {
    if constexpr (!std::is_same_v<T, bracket_value_type<T>>) {
      return traversable_rank<bracket_value_type<T>, CURR_RANK + 1>();
    } else {
      return (rank_t)-1; // infinite RANK
    }
  } else {
    return CURR_RANK;
  }
}

template<typename DS, rank_t KEY_RANK>
inline auto &get_element(DS &element, const ikey<KEY_RANK> &key) {
  static_assert(traversable_rank<DS>() >= KEY_RANK);
  if constexpr (KEY_RANK == 0) {
    return element;
  } else {
    assert(key[0] < element.size());
    return get_element(element[key[0]], *reinterpret_cast<const ikey<KEY_RANK - 1> *>(&key[1]));
  }
}

template<typename DS, rank_t KEY_RANK>
inline const auto &get_element(const DS &element, const ikey<KEY_RANK> &key) {
  static_assert(traversable_rank<DS>() >= KEY_RANK);
  if constexpr (KEY_RANK == 0) {
    return element;
  } else {
    assert(key[0] < element.size());
    return get_element(element[key[0]], *reinterpret_cast<const ikey<KEY_RANK - 1> *>(&key[1]));
  }
}

template<rank_t RANK, typename FUNC, typename DS, rank_t ORG_RANK = RANK>
void apply_nested_ds(DS &ds, FUNC f, dlm::ikey<ORG_RANK> curr_key = {}) {
  if constexpr (RANK == 0) {
    f(curr_key);
  } else {
    for (index_t i = 0;
         i < ds_modifier::size(ds, curr_key.template shrink_to_beg<ORG_RANK - RANK>());
         i++) {
      curr_key[ORG_RANK - RANK] = i;
      apply_nested_ds<RANK - 1>(ds, f, curr_key);
    }
  }
}

template<typename T, rank_t RANK>
using rank_value_type = std::remove_cvref_t<decltype(get_element(std::declval<T>(), ikey<RANK>{}))>;

template<typename DS, rank_t RANK, rank_t CURR_RANK = 0>
bool is_key_valid(const DS &ds, ikey<RANK> key) {
  if constexpr (CURR_RANK == RANK) {
    return true;
  } else {
    index_t size = ds_modifier::size(ds, key.template shrink_to_beg<CURR_RANK>());
    if (key[CURR_RANK] < size) {
      return is_key_valid<DS, RANK, CURR_RANK + 1>(ds, key);
    } else {
      return false;
    }
  }
}

} // namespace dlm
