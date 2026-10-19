#pragma once
#include "basic_types.h"
#include "ckey.h"
#include "ds_flags.h"
#include "ikey.h"
#include <cassert>

namespace dlm {
// An interface between the member operations provided by the data sets, and the
// generalized operations that can be performed on them.
class ds_modifier {
public:
  template<rank_t RANK, typename T>
  inline static void move(T &t, ikey<RANK> dst, ikey<RANK> src) {
    if constexpr (requires { t.move(dst, src); }) {
      t.move(dst, src);
    } else {
      ds_modifier::move(t, t, dst, src);
    }
  }

  // other ds move. Needed by comp_ds, most general move operation
  template<rank_t RANK, typename T>
  static void move(T &dst_ds, T &src_ds, const ikey<RANK> &dst, const ikey<RANK> &src) {
    dst_ds.move(src_ds, dst, src);
  }

  template<rank_t RANK, typename T>
  inline static void swap(T &t, ikey<RANK> a, ikey<RANK> b) {
    if constexpr (requires { t.swap(a, b); }) {
      t.swap(a, b);
    } else {
      ds_modifier::swap(t, t, a, b);
    }
  }

  // other ds swap. Needed by comp_ds, most general swap operation
  template<rank_t RANK, typename T>
  static void swap(T &a_ds, T &b_ds, const ikey<RANK> &a, const ikey<RANK> &b) {
    a_ds.swap(b_ds, a, b);
  }

  template<typename T, rank_t RANK = 0>
  inline static void resize(T &t, index_t new_size, const ikey<RANK> &key = {}) {
    if constexpr (RANK == 0 && requires { t.resize(new_size); }) {
      t.resize(new_size);
    } else {
      t.resize(new_size, key);
    }
  }

  // most fundemental copy operation.
  template<typename T, rank_t RANK>
  static void copy(T &dst_ds, const T &src_ds, const ikey<RANK> &dst, const ikey<RANK> &src) {
    dst_ds.copy(src_ds, dst, src);
  }

  template<rank_t RANK, typename T>
  static void null(T &t, ikey<RANK> key) {
    static_assert(RANK != 0);

    if constexpr (requires { t.null(key); }) {
      t.null(key);
    }
  }

  template<typename T>
  static void clear_null(T &t) {
    if constexpr (requires { t.clear_null(); }) {
      t.clear_null();
    }
  }

  template<typename T, rank_t RANK = 0>
  static void clear(T &t, const ikey<RANK> &key) {
    if constexpr (RANK == 0 && requires { t.clear(); }) {
      t.clear();
    } else if constexpr (requires { t.clear(key); }) {
      t.clear(key);
    } else {
      resize(t, 0, key);
    }
  }

  template<typename T, rank_t RANK = 0>
  static index_t size(const T &t, ikey<RANK> key = {}) {
    if constexpr (RANK == 0 && requires { t.size(); }) {
      return t.size();
    } else {
      return t.size(key);
    }
  }

  template<rank_t P, typename T>
  static auto &get(T &t, ckey<P> c = {}) {
    if constexpr (P == 0 && !requires { t.template get<0>(); }) {
      return t;
    } else {
      return t.template get<P>();
    }
  }

  template<typename T, rank_t RANK>
  static void merge(T &t, const ikey<RANK> &dst, const ikey<RANK> &src) {
    assert(dst != src);

    if constexpr (requires { t.merge(dst, src); }) {
      t.merge(dst, src);
    } else if constexpr (T::rank > RANK) {
      unsigned org_dst_size = size(t, dst);
      resize(t, size(t, dst) + size(t, src), dst);
      for (unsigned i = 0; i < size(t, src); i++) {
        move(t, dst.add_end(org_dst_size + i), src.add_end(i));
      }
      resize(t, 0, src);
    }
  }

  template<typename DS, unsigned char KEY_RANK>
  static void reinsert(
    DS &ds,
    const dlm::ikey<KEY_RANK> &key,
    const unsigned dst,
    const unsigned src
  ) {
    if (dst < src) {
      for (unsigned i = 0; i < src - dst; i++) {
        ds_modifier::swap(ds, key.add_end(src - i), key.add_end(src - i - 1));
      }
    } else {
      for (unsigned i = 0; i < dst - src; i++) {
        ds_modifier::swap(ds, key.add_end(src + i), key.add_end(src + i + 1));
      }
    }
  }
};
} // namespace dlm
