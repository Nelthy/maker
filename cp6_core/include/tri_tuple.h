#pragma once
#include "tri_utils.h"
#include <tuple>
#include <utility>

template<unsigned DIM, template<unsigned, unsigned> class GEN_T>
class tri_tuple {
  template<unsigned R>
  static constexpr auto tri_tuple_helper() {
    return []<size_t... Cs>(std::index_sequence<Cs...>) {
      return std::tuple<typename GEN_T<R, R + Cs>::type...>{};
    }(std::make_index_sequence<DIM - R>{});
  }

public:
  template<unsigned R, unsigned C>
  auto &get() {
    static_assert(R <= C);
    return std::get<tri_linearize(R, C, DIM)>(m_tuple);
  }

  template<unsigned R, unsigned C>
  const auto &get() const {
    static_assert(R <= C);
    return std::get<tri_linearize(R, C, DIM)>(m_tuple);
  }

  std::decay_t<decltype([]<size_t... Rs>(std::index_sequence<Rs...>) {
    return std::tuple_cat(tri_tuple_helper<Rs>()...);
  }(std::make_index_sequence<DIM>{}))>
    m_tuple;

private:
};
