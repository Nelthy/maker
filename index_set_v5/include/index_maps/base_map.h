#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include "get.h"
#include "operation_fn_ptrs.h"
#include <vector>

namespace dlm {
// provides a virtual interface for the inverse to access.
template<rank_t RANK, rank_t INV_RANK>
struct map_interface {
protected:
  // so the inverse can be accessed
  template<typename T1>
  friend class base_map;

  template<typename T1>
  friend class multi_base_map;

  template<typename T1>
  friend class multi_dmap;

  map_interface<INV_RANK, RANK> *inverse_interface;
  size_fn<RANK - 1> m_size_fn;
  grow_fn<RANK - 1> m_grow_fn;
  clear_null_fn m_clear_null_fn;
  void *m_root_perm = nullptr;

  virtual void set_inverse(ikey<RANK> key, ikey<INV_RANK> value) = 0;

public:
  virtual void unlink(ikey<RANK> key) = 0;
  virtual bool is_0_valid(ikey<RANK> key) = 0;

  template<rank_t... Ps, typename IS>
  void init(
    IS &self_is,
    ckey<Ps...> self_perm,
    map_interface<INV_RANK, RANK> &inverse_ds
  ) {
    m_root_perm = &self_is;
    this->inverse_interface = &inverse_ds;
    m_grow_fn = get_grow_fn<Ps...>(self_is);
    m_size_fn = get_size_fn<Ps...>(self_is);
    m_clear_null_fn = get_clear_null_fn(self_is);
  }
};

template<typename M1, typename M2, typename P1, typename P2>
void init_maps(M1 &map1_is, M2 &map2_is, P1 map1_ds_key, P2 map2_ds_key) {
  get(map1_is, map1_ds_key)
    .init(map1_is, map1_ds_key.cut_end(), get(map2_is, map2_ds_key));
  get(map2_is, map2_ds_key)
    .init(map2_is, map2_ds_key.cut_end(), get(map1_is, map1_ds_key));
}

// STANDARD:
// STORE_T is expected to have the following functions exposed to base_map
//  set_inverse
//  get_inverse
//  is_0_valid
//  swap_keys
//  STORE_T must also be a valid data set

//////////////////////////////////////////////////////////////////////////////////////
// STORE_T: key storage type
template<typename STORE_T>
class base_map:
protected STORE_T,
  public map_interface<STORE_T::rank, STORE_T::inv_rank> {
public:
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr rank_t inv_rank = STORE_T::inv_rank;
  using interface_t = map_interface<STORE_T::rank, STORE_T::inv_rank>;
  using STORE_T::get_inverse;
  using STORE_T::is_0_valid;
  using STORE_T::size;

  // directly links two indices.
  bool link(const ikey<rank> &a, const ikey<inv_rank> &b) {
    auto &inverse = *this->inverse_interface;
    // cannot link already linked indices
    assert(!inverse.is_0_valid(b));
    assert(!is_0_valid(a));

    // both keys are not linked, so we can link them
    STORE_T::set_inverse(a, b);
    inverse.set_inverse(b, a);

    return true;
  }

  // links indices. If an index is rank-1, then it creates a new index in rank
  // and links that
  template<rank_t RANK_MAP, rank_t RANK_INV>
  bool sublink(const ikey<RANK_MAP> &a, const ikey<RANK_INV> &b) {
    ikey<rank> end_key_a;
    ikey<inv_rank> end_key_b;
    auto &inverse = *this->inverse_interface;
    assert(this->m_root_perm);
    assert(inverse.m_root_perm);

    static_assert(rank - RANK_MAP == 1 || rank == RANK_MAP);
    static_assert(inv_rank - RANK_INV == 1 || inv_rank == RANK_INV);

    if constexpr (rank == RANK_MAP) {
      // if the keys are of the same rank, link them directly
      end_key_a = a;
    } else if constexpr (rank - RANK_MAP == 1) {
      // if the key is one rank lower, add an index for it
      end_key_a = a.add_end(size(a));
      this->m_grow_fn(this->m_root_perm, 1, a);
    }

    if constexpr (inv_rank == RANK_INV) {
      // if the keys are of the same rank, link them directly
      end_key_b = b;
    } else if constexpr (inv_rank - RANK_INV == 1) {
      // if the key is one rank lower, add an index for it
      end_key_b = b.add_end(inverse.m_size_fn(inverse.m_root_perm, b));
      inverse.m_grow_fn(inverse.m_root_perm, 1, b);
    }

    return link(end_key_a, end_key_b);
  }

  // swaps the contents of the src vector and the empty dst rank keys and links
  // them.
  template<rank_t INV_RANK>
  void swap_link(std::vector<ikey<INV_RANK>> &src, ikey<rank - 1> dst) {
    auto &inverse = *this->inverse;
    assert(this->m_root_perm);
    assert(inverse.m_root_perm);
    assert(ds_modifier::size(*static_cast<STORE_T *>(this), dst) == 0);

    index_t key_num = src.size();
    STORE_T::swap_keys(src, dst);
    // so the index set is aligned with the newly added keys
    this->m_grow_fn(this->m_root_perm, key_num, dst);
    set_inverse(dst, dst);
  }

  template<rank_t KEY_RANK>
  const auto &operator[](const ikey<KEY_RANK> &key) const {
    return STORE_T::operator[](key);
  }

  const auto &operator[](index_t key) const { return STORE_T::operator[](key); }

  const auto &get_store() const { return static_cast<const STORE_T &>(*this); }

  // DANGER: Only use if you know what you're doing!!
  auto &get_store() { return static_cast<STORE_T &>(*this); }

  template<typename... ARGs>
  void init_store(ARGs &&...args) {
    STORE_T::init_store(args...);
  }

protected:
  friend class ds_modifier;

  template<rank_t KEY_RANK>
  void null(const ikey<KEY_RANK> &key) {
    static_assert(KEY_RANK <= rank);
    assert(key.is_valid());
    if constexpr (KEY_RANK == rank) {
      if (is_0_valid(key)) {
        ikey<inv_rank> inv_key = get_inverse(key);
        STORE_T::set_inverse(
          key,
          {}
        ); // nulling before unlinking stops circular dependencies
        this->inverse_interface->unlink(inv_key);
      }
    } else {
      for (index_t i = 0; i < size(key); i++) { null(key.add_end(i)); }
    }
  }

private:
  template<rank_t KEY_RANK>
  void set_inverse_key(ikey<KEY_RANK> key, ikey<KEY_RANK> new_inverse) {
    assert(key.is_valid());
    assert(new_inverse.is_valid());
    if constexpr (KEY_RANK == rank) {
      if (is_0_valid(key)) {
        this->inverse_interface->set_inverse(get_inverse(key), new_inverse);
      }
    } else {
      for (index_t i = 0; i < size(key); i++) {
        set_inverse_key(key.add_end(i), new_inverse.add_end(i));
      }
    }
  }

  template<rank_t KEY_RANK>
  void move(const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
    static_assert(KEY_RANK <= rank);
    set_inverse_key(src, dst);
    ds_modifier::move(*static_cast<STORE_T *>(this), dst, src);
  }

  template<rank_t KEY_RANK>
  void swap(const ikey<KEY_RANK> &a, const ikey<KEY_RANK> &b) {
    static_assert(KEY_RANK <= rank);
    set_inverse_key(a, b);
    ds_modifier::swap(*static_cast<STORE_T *>(this), a, b);
    set_inverse_key(a, a);
  }

  void clear_null() {
    assert(this->inverse_interface);
    // this ensures that all connected maps get flushed too
    this->inverse_interface->m_clear_null_fn(
      this->inverse_interface->m_root_perm
    );
  }

  template<rank_t KEY_RANK>
  void clear(const ikey<KEY_RANK> &key) {
    static_assert(KEY_RANK < rank);
    static_assert(KEY_RANK < rank, "operation rank too high");
    for (index_t i = 0; i < size(key); i++) { null(key.add_end(i)); }
    this->inverse_interface->m_clear_null_fn(
      this->inverse_interface->m_root_perm
    );
    ds_modifier::clear(*static_cast<STORE_T *>(this), key);
  }

  // template<rank_t... Ps, typename IS, rank_t INV_RANK>
  // friend void swap_link(
  //   IS &is,
  //   std::vector<ikey<INV_RANK>> &src,
  //   ikey<get_t<IS, ckey<Ps...>>::rank - 1> dst,
  //   ckey<Ps...> map_n
  // );
  //
  // template<typename MAP_A_N, typename MAP_B_N, typename IS_A, typename IS_B>
  // friend bool sublink(
  //   IS_A &is_a,
  //   IS_B &is_b,
  //   const ikey<get_t<IS_A, MAP_A_N>::rank - 1> &a,
  //   const ikey<get_t<IS_B, MAP_B_N>::rank - 1> &b,
  //   MAP_A_N map_a_n,
  //   MAP_B_N map_b_n
  // );
};

//
// template<rank_t... Ps, typename IS, rank_t INV_RANK>
// void swap_link(
//   IS &is,
//   std::vector<ikey<INV_RANK>> &src,
//   ikey<get_t<IS, ckey<Ps...>>::rank - 1> dst,
//   ckey<Ps...> map_n
// ) {
//   auto &map = get<Ps...>(is);
//   auto &inverse = *map.inverse_interface;
//   assert(inverse.m_root);
//   assert(ds_modifier::size(map, dst) == 0);
//
//   index_t key_num = src.size();
//   map.swap_keys(src, dst);
//   // so the index set is aligned with the newly added keys
//   grow(is, key_num, dst, map_n);
//   map.set_inverse(dst, dst);
// }
//
// template<rank_t... Ps, typename IS>
// bool sublink(
//   IS &is,
//   const ikey<get_t<IS, ckey<Ps...>>::rank - 1> &a,
//   const ikey<get_t<IS, ckey<Ps...>>::inv_rank> &b,
//   ckey<Ps...> map_n
// ) {
//   grow(is, 1, a, map_n.cut_end());
//   return get(is, map_n).link(a.add_end(size(is, a, map_n.cut_end()) - 1), b);
// }
//
// template<typename MAP_A_N, typename MAP_B_N, typename IS_A, typename IS_B>
// bool sublink(
//   IS_A &is_a,
//   IS_B &is_b,
//   const ikey<get_t<IS_A, MAP_A_N>::rank - 1> &a,
//   const ikey<get_t<IS_B, MAP_B_N>::rank - 1> &b,
//   MAP_A_N map_a_n,
//   MAP_B_N map_b_n
// ) {
//   assert(get(is_a, map_a_n).inverse_interface == &get(is_b, map_b_n));
//
//   grow(is_a, 1, a, map_a_n.cut_end());
//   grow(is_b, 1, b, map_b_n.cut_end());
//   return get(is_a, map_a_n)
//     .link(
//       a.add_end(size(is_a, a, map_a_n.cut_end()) - 1),
//       b.add_end(size(is_b, b, map_b_n.cut_end()) - 1)
//     );
// }
//
} // namespace dlm
