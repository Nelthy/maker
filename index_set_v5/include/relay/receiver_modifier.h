#pragma once
#include "ikey.h"
#include <cassert>

// for a class to work with receiver modifier, they must, at least, have these three functions
// overloaded
template<rank_t RANK, typename T>
inline void on_resize_overload(T &rec, index_t new_size, dlm::ikey<RANK> key) {};

template<rank_t RANK, typename T>
inline void on_move_overload(T &dst_rec, T &src_rec, dlm::ikey<RANK> dst, dlm::ikey<RANK> src) {};

template<rank_t RANK, typename T>
inline void on_swap_overload(T &a_rec, T &b_rec, dlm::ikey<RANK> a, dlm::ikey<RANK> b) {};

template<rank_t RANK, typename T>
inline void on_null_overload(T &ds, dlm::ikey<RANK> key) {}

template<typename T>
inline void on_clear_null_overload(T &ds) {}

namespace dlm {
// An interface between the operations provided by the data sets, and the generalized
// operations that can be performed on them.
class receiver_modifier {
public:
  template<rank_t RANK, typename T>
  inline static void on_move(T &rec, ikey<RANK> dst, ikey<RANK> src) {
    if constexpr (requires { rec.on_move(dst, src); }) {
      rec.on_move(dst, src);
    } else {
      receiver_modifier::on_move(rec, rec, dst, src);
    }
  }

  template<rank_t RANK, typename T>
  inline static void on_swap(T &rec, ikey<RANK> a, ikey<RANK> b) {
    if constexpr (requires { rec.on_swap(a, b); }) {
      rec.on_swap(a, b);
    } else {
      receiver_modifier::on_swap(rec, rec, a, b);
    }
  }

  template<rank_t RANK, typename T>
  inline static void on_resize(T &rec, index_t new_size, const ikey<RANK> &key) {
    if constexpr (RANK == 0 && requires { rec.on_resize(new_size); }) {
      rec.on_resize(new_size);
    } else if constexpr (requires { rec.on_resize(new_size, key); }) {
      rec.on_resize(new_size, key);
    } else {
      on_resize_overload(rec, new_size, key);
    }
  }

  template<rank_t RANK, typename T>
  static void on_move(T &dst_rec, T &src_rec, const ikey<RANK> &dst, const ikey<RANK> &src) {
    if constexpr (requires { dst_rec.on_move_from(src_rec, dst, src); }) {
      dst_rec.on_move_from(src_rec, dst, src);
    } else {
      on_move_overload(dst_rec, src_rec, dst, src);
    }
  }

  template<rank_t RANK, typename T>
  static void on_swap(T &a_rec, T &b_rec, const ikey<RANK> &a, const ikey<RANK> &b) {
    if constexpr (requires { a_rec.on_swap_from(b_rec, a, b); }) {
      a_rec.on_swap_from(b_rec, a, b);
    } else {
      on_swap_overload(a_rec, b_rec, a, b);
    }
  }

  template<rank_t RANK, typename T>
  static void on_null(T &rec, ikey<RANK> key) {
    static_assert(RANK != 0);

    if constexpr (requires { rec.on_null(key); }) {
      rec.on_null(key);
    } else {
      on_null_overload(rec, key);
    }
  }

  template<typename T>
  static void on_clear_null(T &rec) {
    if constexpr (requires { rec.on_clear_null(); }) {
      rec.on_clear_null();
    } else {
      on_clear_null_overload(rec);
    }
  }

  template<typename T, rank_t RANK = 0>
  static void on_clear(T &t, const ikey<RANK> &key) {
    if constexpr (RANK == 0 && requires { t.clear(); }) {
      t.on_clear();
    } else if constexpr (requires { t.clear(key); }) {
      t.on_clear(key);
    } else {
      on_resize(t, 0, key);
    }
  }
  template<rank_t RANK, typename T>
  inline static void on_merge(T &rec, ikey<RANK> dst, ikey<RANK> src) {
    if constexpr (requires { rec.on_merge(dst, src); }) {
      rec.on_merge(dst, src);
    }
  }
};

} // namespace dlm
