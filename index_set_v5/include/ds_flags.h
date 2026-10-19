#pragma once
#include "basic_types.h"
#include "ikey.h"

namespace dlm {
template<typename T>
concept has_rank = requires(T t) { T::rank; };

template<typename T, rank_t RANK = 1>
concept has_bracket = requires(T t) { t[ikey<RANK>{}]; };

template<typename T, rank_t RANK = 1>
using bracket_value_type = std::remove_cvref_t<decltype(std::declval<T>()[ikey<RANK>{}])>;

// comp stands for composition
template<typename T>
concept is_comp = requires(T t) {
  requires(T::is_comp);
  requires(has_rank<T>);
  requires(has_bracket<T, T::rank>);
};
} // namespace dlm
