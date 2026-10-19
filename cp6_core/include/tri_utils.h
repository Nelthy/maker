#pragma once
#include <cassert>
#include <utility>

constexpr inline unsigned tri_linearize(unsigned r, unsigned c, unsigned dim) {
  assert(r <= c);
  return ((dim * r) - (r * (r + 1) / 2)) + (c);
}

constexpr inline unsigned tri_num(unsigned dim) { return tri_linearize(dim - 1, dim - 1, dim) + 1; }

// needed because fold expressions are dumb
template<size_t I, typename FUNCTOR, size_t... Js>
constexpr inline void tri_apply_helper(FUNCTOR f, std::index_sequence<Js...>) {
  (f.template operator()<I, Js + I>(), ...);
};

/*
  a compile time version of:
  for(unsigned i = 0; i < DIM; i++){
    for(unsigned j = i + 1; j < DIM; j++){
      FUNCTOR(i, j);
    }
  }
  useful for iterating over triangular matrices. Triangular matrices are useful
  for modeling combinations of a set's indices.
*/
template<size_t DIM, typename FUNCTOR>
constexpr inline void tri_apply(FUNCTOR f) {
  [f]<size_t... Is>(std::index_sequence<Is...>) {
    (tri_apply_helper<Is>(f, std::make_index_sequence<DIM - Is>{}), ...);
  }(std::make_index_sequence<DIM>());
}

// function to redirect the main thread to do work instead of spinning. Must be defined by user.
void on_main_spin();

// spins, updating non true indices, until all indices have returned a true value
template<size_t DIM, typename FUNCTOR>
constexpr inline void tri_apply_spin(FUNCTOR f) {
  bool is_done[tri_num(DIM)]{};
  unsigned done_num = 0;
  while (done_num < tri_num(DIM)) {
    tri_apply<DIM>([&]<size_t R, size_t C> {
      if (!is_done[tri_linearize(R, C, DIM)]) {
        if (f.template operator()<R, C>()) {
          is_done[tri_linearize(R, C, DIM)] = true;
          done_num++;
        };
      }
    });
    on_main_spin();
  }
}
