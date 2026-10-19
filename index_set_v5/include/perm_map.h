#pragma once

#include "basic_types.h"
#include "ckey.h"
#include "ds_modifier.h"
#include "merge_keys.h"
#include "operation_fn_ptrs.h"

namespace dlm {

struct ext_fns {
private:
  template<rank_t RANK, rank_t RT_RANK>
  friend class perm_map;

  template<rank_t RANK, rank_t... IND>
  friend class perm_map_merge;

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void resize_thunk(void *t, unsigned new_size, const ikey<KEY_RANK> &key) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_resize(new_size, key);
  }

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void move_thunk(void *t, const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_move(dst, src);
  }

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void swap_thunk(void *t, const ikey<KEY_RANK> &a, const ikey<KEY_RANK> &b) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_swap(a, b);
  }

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void null_thunk(void *t, const ikey<KEY_RANK> &key) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_null(key);
  }

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void merge_thunk(void *t, const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_merge(dst, src);
  }

  template<typename T, rank_t KEY_RANK, rank_t... Ps>
  static void clear_thunk(void *t, const ikey<KEY_RANK> &key) {
    get<Ps...>(*reinterpret_cast<T *>(t)).ext_clear(key);
  }
};

// Maps a permute of one index set to the permute of another
// RT_RANK is the size of the run time key.
template<rank_t RANK, rank_t RT_RANK = 0>
class perm_map {
public:
  static constexpr rank_t rank = RANK;

  perm_map() {};

  template<typename LINKED_IS, rank_t... Ps>
  perm_map(LINKED_IS &linked_is, ckey<Ps...> c = {}, ikey<RT_RANK> linked_rt_key = {}) {
    static_assert(LINKED_IS::rank == 0);
    init(linked_is, c, linked_rt_key);
  }

  template<typename LINKED_IS, rank_t... Ps>
  void init(LINKED_IS &linked_is, ckey<Ps...> c = {}, ikey<RT_RANK> linked_rt_key = {}) {
    static_assert(RT_RANK + rank == sizeof...(Ps) + 1);
    assert(linked_rt_key.is_valid());

    this->linked_is = &linked_is;
    this->linked_rt_key = linked_rt_key;
    m_resize_fn = ext_fns::template resize_thunk<LINKED_IS, RANK + RT_RANK - 1, Ps...>;
    m_move_fn = ext_fns::template move_thunk<LINKED_IS, RANK + RT_RANK, Ps...>;
    m_swap_fn = ext_fns::template swap_thunk<LINKED_IS, RANK + RT_RANK, Ps...>;
    m_null_fn = ext_fns::template null_thunk<LINKED_IS, RANK + RT_RANK, Ps...>;
    m_clear_null_fn = clear_null_thunk<LINKED_IS>;
    m_merge_fn = ext_fns::template merge_thunk<LINKED_IS, RANK + RT_RANK, Ps...>;
    m_clear_fn = ext_fns::template clear_thunk<LINKED_IS, RANK + RT_RANK - 1, Ps...>;
  }

private:
  friend class ds_modifier;

  void (*m_resize_fn)(void *t, unsigned new_size, const ikey<RANK + RT_RANK - 1> &key);
  void (*m_clear_fn)(void *t, const ikey<RANK + RT_RANK - 1> &key);
  void (*m_move_fn)(void *t, const ikey<RANK + RT_RANK> &dst, const ikey<RANK + RT_RANK> &src);
  void (*m_swap_fn)(void *t, const ikey<RANK + RT_RANK> &a, const ikey<RANK + RT_RANK> &b);
  void (*m_null_fn)(void *t, const ikey<RANK + RT_RANK> &key);
  void (*m_clear_null_fn)(void *t);
  void (*m_merge_fn)(void *t, const ikey<RANK + RT_RANK> &dst, const ikey<RANK + RT_RANK> &src);

  void *linked_is = nullptr;
  ikey<RT_RANK> linked_rt_key;

  template<rank_t KEY_RANK>
  void resize(index_t new_size, ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == rank - 1) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_resize_fn(linked_is, new_size, linked_rt_key.add_end(key));
    }
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == rank - 1) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_clear_fn(linked_is, linked_rt_key.add_end(key));
    }
  }

  template<rank_t KEY_RANK>
  void move(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_move_fn(linked_is, linked_rt_key.add_end(dst), linked_rt_key.add_end(src));
    }
  }

  template<rank_t KEY_RANK>
  void swap(ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_swap_fn(linked_is, linked_rt_key.add_end(a), linked_rt_key.add_end(b));
    }
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> removed_key) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_null_fn(linked_is, linked_rt_key.add_end(removed_key));
    }
  }

  void clear_null() { m_clear_null_fn(linked_is); }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_merge_fn(linked_is, linked_rt_key.add_end(dst), linked_rt_key.add_end(src));
    }
  }
};

// Maps a permute of one index set to the permute of another
// Merges the runtime key with the input key using the merge fucntion. 
template<rank_t RANK, rank_t... IND>
class perm_map_merge {
public:
  static constexpr rank_t rank = RANK;
  static constexpr rank_t rt_rank = sizeof...(IND) - rank;
  static constexpr auto ind = ckey<IND...>{};

  perm_map_merge() {};

  template<typename LINKED_IS, rank_t... Ps>
  perm_map_merge(LINKED_IS &linked_is, ckey<Ps...> c = {}, ikey<rt_rank> linked_rt_key = {}) {
    static_assert(LINKED_IS::rank == 0);
    init(linked_is, c, linked_rt_key);
  }

  template<rank_t... Ps, typename LINKED_IS>
  void init(LINKED_IS &linked_is, ckey<Ps...> c = {}, ikey<rt_rank> linked_rt_key = {}) {
    static_assert(rt_rank + rank == sizeof...(Ps) + 1);
    assert(linked_rt_key.is_valid());

    this->linked_is = &linked_is;
    this->linked_rt_key = linked_rt_key;
    m_resize_fn = ext_fns::template resize_thunk<LINKED_IS, RANK + rt_rank - 1, Ps...>;
    m_move_fn = ext_fns::template move_thunk<LINKED_IS, RANK + rt_rank, Ps...>;
    m_swap_fn = ext_fns::template swap_thunk<LINKED_IS, RANK + rt_rank, Ps...>;
    m_null_fn = ext_fns::template null_thunk<LINKED_IS, RANK + rt_rank, Ps...>;
    m_clear_null_fn = clear_null_thunk<LINKED_IS>;
    m_merge_fn = ext_fns::template merge_thunk<LINKED_IS, RANK + rt_rank, Ps...>;
    m_clear_fn = ext_fns::template clear_thunk<LINKED_IS, RANK + rt_rank - 1, Ps...>;
  }

private:
  friend class ds_modifier;

  void (*m_resize_fn)(void *t, unsigned new_size, const ikey<RANK + rt_rank - 1> &key);
  void (*m_clear_fn)(void *t, const ikey<RANK + rt_rank - 1> &key);
  void (*m_move_fn)(void *t, const ikey<RANK + rt_rank> &dst, const ikey<RANK + rt_rank> &src);
  void (*m_swap_fn)(void *t, const ikey<RANK + rt_rank> &a, const ikey<RANK + rt_rank> &b);
  void (*m_null_fn)(void *t, const ikey<RANK + rt_rank> &key);
  void (*m_clear_null_fn)(void *t);
  void (*m_merge_fn)(void *t, const ikey<RANK + rt_rank> &dst, const ikey<RANK + rt_rank> &src);

  void *linked_is = nullptr;
  ikey<rt_rank> linked_rt_key;

  template<rank_t KEY_RANK>
  void resize(index_t new_size, ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == rank - 1) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_resize_fn(linked_is, new_size, merge_keys(linked_rt_key, key, ind.cut_end()));
    }
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == rank - 1) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_clear_fn(linked_is, merge_keys(linked_rt_key, key, ind.cut_end()));
    }
  }

  template<rank_t KEY_RANK>
  void move(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_move_fn(
        linked_is,
        merge_keys(linked_rt_key, dst, ind),
        merge_keys(linked_rt_key, src, ind)
      );
    }
  }

  template<rank_t KEY_RANK>
  void swap(ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_swap_fn(linked_is, merge_keys(linked_rt_key, a, ind), merge_keys(linked_rt_key, b, ind));
    }
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> removed_key) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_null_fn(linked_is, merge_keys(linked_rt_key, removed_key, ind));
    }
  }

  void clear_null() { m_clear_null_fn(linked_is); }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      assert(linked_is && "must initialize perm map before calling operations");
      m_merge_fn(
        linked_is,
        merge_keys(linked_rt_key, dst, ind),
        merge_keys(linked_rt_key, src, ind)
      );
    }
  }
};
} // namespace dlm
