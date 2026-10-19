#pragma once
#include "ikey.h"
#include <cassert>

namespace dlm {
template<typename PERMUTE>
class external_perm: public PERMUTE {
private:
  friend class ds_modifier;
  friend struct ext_fns;

  template<rank_t KEY_RANK = 0>
  void resize(index_t new_size, ikey<KEY_RANK> key = {}) {
    if constexpr (KEY_RANK == PERMUTE::rank) {
      // local operation called on external perm
      assert(false && "local operation called on external perm");
    } else {
      PERMUTE::resize(new_size, key);
    }
  }

  template<rank_t KEY_RANK>
  void move(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == PERMUTE::rank + 1) {
      // local operation called on external perm
      assert(false && "local operation called on external perm");
    } else {
      PERMUTE::move(dst, src);
    }
  }

  template<rank_t KEY_RANK>
  void swap(ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    if constexpr (KEY_RANK == PERMUTE::rank + 1) {
      assert(false && "local operation called on external perm");
    } else {
      PERMUTE::swap(a, b);
    }
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == PERMUTE::rank + 1) {
      // local operation called on external perm
      assert(false);
    } else {
      PERMUTE::null(key);
    }
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == PERMUTE::rank + 1) {
      // local operation called on external perm
      assert(false);
    } else {
      PERMUTE::merge(dst, src);
    }
  }

  template<rank_t KEY_RANK = 0>
  void clear(ikey<KEY_RANK> key = {}) {
    if constexpr (KEY_RANK == PERMUTE::rank) {
      // local operation called on external perm
      assert(false);
    } else {
      PERMUTE::clear(key);
    }
  }

  void ext_resize(index_t new_size, ikey<PERMUTE::rank> key = {}) {
    PERMUTE::resize(new_size, key);
  }

  // these functions are called directly from perm_map
  void ext_move(ikey<PERMUTE::rank + 1> dst, ikey<PERMUTE::rank + 1> src) {
    PERMUTE::move(dst, src);
  }

  void ext_swap(ikey<PERMUTE::rank + 1> a, ikey<PERMUTE::rank + 1> b) { PERMUTE::swap(a, b); }

  void ext_null(ikey<PERMUTE::rank + 1> key) { PERMUTE::null(key); }

  void ext_merge(ikey<PERMUTE::rank + 1> dst, ikey<PERMUTE::rank + 1> src) {
    PERMUTE::merge(dst, src);
  }

  void ext_clear(ikey<PERMUTE::rank> key = {}) { PERMUTE::clear(key); }
};

template<typename PERMUTE>
using ext = external_perm<PERMUTE>;

} // namespace dlm
