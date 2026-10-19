#pragma once
#include "ckey.h"
#include "ds_modifier.h"
#include "get.h"
#include "ikey.h"
#include "operation_ids.h"

namespace dlm {
template<rank_t... Ps, typename T>
  requires(T::rank == 0)
index_t size(const T &t, ikey<sizeof...(Ps)> key = {}, ckey<Ps...> c = {}) {
  assert(is_key_valid(get(t, c), key));

  return ds_modifier::size(dlm::get(t, c), key);
}

// grows set by amount. Leaves the new indices undefined
template<rank_t... Ps, typename T>
  requires(T::rank == 0)
void grow(T &t, index_t amount, ikey<sizeof...(Ps)> key = {}, ckey<Ps...> c = {}) {
  assert(t.operation_id == no_operation);
  assert(is_key_valid(get(t, c), key));

  t.operation_id = resize_operation;
  ds_modifier::resize(get(t, c), size(t, key, c) + amount, key);
  t.operation_id = no_operation;
}

// swaps the indices
template<rank_t... Ps, typename T>
  requires(T::rank == 0)
void swap(T &t, ikey<sizeof...(Ps) + 1> a, ikey<sizeof...(Ps) + 1> b, ckey<Ps...> c = {}) {
  assert(t.operation_id == no_operation);
  assert(is_key_valid(get(t, c), a));
  assert(is_key_valid(get(t, c), b));

  t.operation_id = swap_operation;
  ds_modifier::swap(get(t, c), a, b);
  t.operation_id = no_operation;
}

// nulls out an index, making it invalid
template<rank_t... Ps, typename T>
  requires(T::rank == 0)
void null(T &t, ikey<sizeof...(Ps) + 1> key, ckey<Ps...> c = {}) {
  assert(
    t.operation_id == no_operation || t.operation_id == null_operation ||
    t.operation_id == clear_operation
  );
  assert(is_key_valid(get(t, c), key));

  t.operation_id = null_operation;
  ds_modifier::null(get(t, c), key);
}

// clears all the null indices from the set and it's dependencies
template<typename T>
  requires(T::rank == 0)
void clear_null(T &t) {
  // no work needs to be done in these cases
  if (t.operation_id == clear_null_operation || t.operation_id == clear_operation) {
    return;
  }

  assert(t.operation_id == null_operation || t.operation_id == no_operation);

  t.operation_id = clear_null_operation;
  ds_modifier::clear_null(t);
  t.operation_id = no_operation;
}

// clears all the indices from this set
template<rank_t... Ps, typename T>
  requires(T::rank == 0)
void clear(T &t, ikey<sizeof...(Ps)> key = {}, ckey<Ps...> c = {}) {
  assert(t.operation_id == no_operation);
  assert(is_key_valid(get(t, c), key));

  t.operation_id = clear_operation;
  ds_modifier::clear(get(t, c), key);
  t.operation_id = no_operation;
}

// appends src's sub indices to the end of dst's. Resizes src to 0. This applies to all sub
// permutes under Ps...
template<typename T, rank_t... Ps>
  requires(T::rank == 0)
void merge(T &t, ikey<sizeof...(Ps) + 1> dst, ikey<sizeof...(Ps) + 1> src, ckey<Ps...> c = {}) {
  assert(t.operation_id == no_operation);
  assert(is_key_valid(get(t, c), dst));
  assert(is_key_valid(get(t, c), src));

  t.operation_id = merge_operation;
  ds_modifier::merge(get(t, c), dst, src);
  t.operation_id = no_operation;
};

// moves src to the end of dst. Leaves src undefined
template<typename T, rank_t... Ps>
void append(
  T &t,
  const ikey<sizeof...(Ps)> &dst,
  const ikey<sizeof...(Ps) + 1> &src,
  ckey<Ps...> c = {}
) {

  index_t dst_size = size<Ps...>(dst);
  grow<Ps...>(1, dst);
  ds_modifier::move(get(t, c), dst.add_end(dst_size), src);
};

// moves the indices to the right of start(inclusive) to the right by amount, grows by amount.
// Leaves start to start + amount undefined
template<typename T, rank_t... Ps>
void shift_right(
  T &t,
  const ikey<sizeof...(Ps) + 1> &start,
  const index_t amount,
  ckey<Ps...> c = {}
) {

  index_t old_size = size<Ps...>(t, start.cut_end());
  grow<Ps...>(t, amount, start.cut_end());
  ikey<sizeof...(Ps) + 1> curr_key = start;
  curr_key.end() = old_size - 1;
  ikey<sizeof...(Ps) + 1> dst_key = curr_key;
  dst_key.end() += amount;

  for (index_t i = 0; i < size<Ps...>(t, start.cut_end()) - start.end() - amount; i++) {
    ds_modifier::move(get(t, c), dst_key, curr_key);
    curr_key.end() -= 1;
    dst_key.end() -= 1;
  }
}

// inserts indices before dst. Leaves src_start + src_start + amount undefined.
template<typename T, rank_t... Ps>
void insert(
  T &t,
  const ikey<sizeof...(Ps) + 1> &dst,
  const ikey<sizeof...(Ps) + 1> &src_start,
  const index_t amount,
  ckey<Ps...> c = {}
) {

  assert(src_start.end() + amount - 1 < size<Ps...>(src_start.cut_end()));

  shift_right<Ps...>(dst, amount);
  if (dst.cut_end() == src_start.cut_end()) {
    if (dst.end() < src_start.end()) {
      src_start.end() += amount;
    }
  }
  for (index_t i = 0; i < amount; i++) {
    ds_modifier::move(get(t, c), dst, src_start);
    dst.end() += 1;
    src_start.end() += 1;
  }
}
} // namespace dlm
