#pragma once
#include "operations.h"

namespace dlm {

// size
template<unsigned RANK>
using size_fn = unsigned (*)(const void *t, const ikey<RANK> &key);

template<typename T, rank_t KEY_RANK, rank_t... Ps>
  requires(T::rank == 0)
unsigned size_thunk(const void *t, const ikey<KEY_RANK> &key) {
  return size<Ps...>(*reinterpret_cast<const T *>(t), key);
}

template<rank_t... Ps, typename T>
  requires(T::rank == 0)
size_fn<sizeof...(Ps)> get_size_fn(const T &t, ckey<Ps...> c = {}) {
  return size_thunk<T, sizeof...(Ps), Ps...>;
}

// grow
template<unsigned RANK>
using grow_fn = void (*)(void *t, unsigned amount, const ikey<RANK> &key);

template<typename T, rank_t KEY_RANK, rank_t... Ps>
  requires(T::rank == 0)
void grow_thunk(void *t, unsigned amount, const ikey<KEY_RANK> &key) {
  grow<Ps...>(*reinterpret_cast<T *>(t), amount, key);
}

template<rank_t... Ps, typename T>
  requires(T::rank == 0)
grow_fn<sizeof...(Ps)> get_grow_fn(T &t, ckey<Ps...> c = {}) {
  return grow_thunk<T, sizeof...(Ps), Ps...>;
}

// swap
template<unsigned RANK>
using swap_fn = void (*)(void *t, const ikey<RANK> &a, const ikey<RANK> &b);

template<typename T, rank_t KEY_RANK, rank_t... Ps>
  requires(T::rank == 0)
void swap_thunk(void *t, const ikey<KEY_RANK> &a, const ikey<KEY_RANK> &b) {
  swap<Ps...>(*reinterpret_cast<T *>(t), a, b);
}

template<rank_t... Ps, typename T>
  requires(T::rank == 0)
swap_fn<sizeof...(Ps) + 1> get_swap_fn(T &t, ckey<Ps...> c = {}) {
  return swap_thunk<T, sizeof...(Ps) + 1, Ps...>;
}

// null
template<unsigned RANK>
using null_fn = void (*)(void *t, const ikey<RANK> &key);

template<typename T, rank_t KEY_RANK, rank_t... Ps>
  requires(T::rank == 0)
void null_thunk(void *t, const ikey<KEY_RANK> &key) {
  null<Ps...>(*reinterpret_cast<T *>(t), key);
};

template<rank_t... Ps, typename T>
null_fn<sizeof...(Ps) + 1> get_null_fn(T &t, ckey<Ps...> c = {}) {
  static_assert(T::rank == 0);
  return null_thunk<T, sizeof...(Ps) + 1, Ps...>;
}

// clear_null
using clear_null_fn = void (*)(void *t);

template<typename T>
void clear_null_thunk(void *t) {
  static_assert(T::rank == 0);
  clear_null(*reinterpret_cast<T *>(t));
}

template<typename T>
  requires(T::rank == 0)
clear_null_fn get_clear_null_fn(T &t) {
  return clear_null_thunk<T>;
}

// merge
template<unsigned RANK>
using merge_fn = void (*)(void *t, const ikey<RANK> &dst, const ikey<RANK> &src);

template<typename T, rank_t KEY_RANK, rank_t... Ps>
  requires(T::rank == 0)
void merge_thunk(void *t, const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
  merge<Ps...>(*reinterpret_cast<T *>(t), dst, src);
}

template<rank_t... Ps, typename T>
  requires(T::rank == 0)
merge_fn<sizeof...(Ps) + 1> get_merge_fn(T &t, ckey<Ps...> c = {}) {
  return merge_thunk<T, sizeof...(Ps) + 1, Ps...>;
}

// inline void foo() {
//   perm<perm<vec_ds<float, 2>>> p;
//   size_fn<1> size = get_size_fn<0>(p);
//   grow_fn<1> grow = get_grow_fn<0>(p);
//   swap_fn<2> swap = get_swap_fn<0>(p);
//   null_fn<2> null = get_null_fn<0>(p);
//   clear_null_fn clear_null = get_clear_null_fn(p);
//   merge_fn<1> merge = get_merge_fn(p);
// };

// // appends src's sub indices to the end of dst's. Resizes src to 0. This applies to all sub
// // permutes under Ps...
// template<typename T, rank_t... Ps>
//   requires(T::rank == 0)
// void merge(T &t, ikey<sizeof...(Ps) + 1> dst, ikey<sizeof...(Ps) + 1> src, ckey<Ps...> c = {}) {
//   assert(is_key_valid(get(t, c), dst));
//   assert(is_key_valid(get(t, c), src));
//
//   ds_modifier::merge(get(t, c), dst, src);
// };
//
} // namespace dlm
