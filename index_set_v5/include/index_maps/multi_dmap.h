#pragma once
#include "multi_base_map.h"
#include "operation_fn_ptrs.h"

namespace dlm {
template<typename STORE_T>
class multi_dmap;

template<rank_t RANK, rank_t INV_RANK, typename STORE_T, index_t I>
struct multi_dmap_interface: map_interface<RANK, INV_RANK> {
private:
  template<typename _STORE_T>
  friend class multi_dmap;

  void set_inverse(ikey<RANK> key, ikey<INV_RANK> value) override {
    m_map->template set_inverse<I>(key, value);
  };

  multi_dmap<STORE_T> *m_map;

public:
  void unlink(ikey<RANK> key) override {
    // this store needs to be dmap or imap here.
    m_map->template unlink<I>(key);
  };

  bool is_0_valid(ikey<RANK> key) override { return m_map->template is_0_valid<I>(key); };
};

// dmap stands for dependent map.
// Every index is dependent on its inverse, so when the inverse unlinks from the index, the index
// gets removed
template<typename STORE_T>
class multi_dmap: public multi_base_map<STORE_T> {
public:
  using base = multi_base_map<STORE_T>;
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr auto inv_ranks = STORE_T::inv_ranks;
  static constexpr index_t map_num = STORE_T::map_num;

  template<index_t I>
  auto &get_interface() {
    return get<I>(m_interfaces);
  }

  multi_dmap() {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      ((get_interface<Is>().m_map = this), ...);
    }(std::make_index_sequence<map_num>{});
  }

  template<index_t I>
  void unlink(ikey<rank> key) {
    m_null_fn(get_interface<0>().m_root_perm, key);
  }

  template<index_t I>
  bool is_0_valid(ikey<rank> key) {
    return STORE_T::template is_0_valid<I>(key);
  }

  // links indices. If an index is rank-1, then it creates a new index in rank and links that
  template<index_t I, rank_t RANK_INV>
  bool sublink(const ikey<rank> &a, const ikey<RANK_INV> &b) {
    ikey<rank> end_key_a;
    ikey<inv_ranks[I]> end_key_b;
    auto &inverse = *get<I>(base::m_inverse_interfaces);
    assert(get<I>(m_interfaces).m_root_perm);
    assert(inverse.m_root_perm);

    static_assert(inv_ranks[I] - RANK_INV == 1 || inv_ranks[I] == RANK_INV);

    end_key_a = a;

    if constexpr (inv_ranks[I] == RANK_INV) {
      // if the keys are of the same rank, link them directly
      end_key_b = b;
    } else if constexpr (inv_ranks[I] - RANK_INV == 1) {
      // if the key is one rank lower, add an index for it
      end_key_b = b.add_end(inverse.m_size_fn(inverse.m_root_perm, b));
      inverse.m_grow_fn(inverse.m_root_perm, 1, b);
    }

    return base::template link<I>(end_key_a, end_key_b);
  }

  template<index_t I, rank_t... Ps, typename IS>
  void init(IS &self_is, ckey<Ps...> self_perm, map_interface<inv_ranks[I], rank> &inverse_ds) {
    get_interface<I>().init(self_is, self_perm, inverse_ds);
    get<I>(base::m_inverse_interfaces) = &inverse_ds;
    if (!m_null_fn) {
      m_null_fn = get_null_fn(self_is, self_perm);
    } else {
      assert((m_null_fn == get_null_fn(self_is, self_perm)));
    }
  }

private:
  template<rank_t RANK, rank_t INV_RANK, typename _STORE_T, index_t I>
  friend struct multi_dmap_interface;

  decltype([]<size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple<multi_dmap_interface<rank, inv_ranks[Is], STORE_T, Is>...>{};
  }(std::make_index_sequence<map_num>{})) m_interfaces;

  null_fn<rank> m_null_fn = nullptr;

  template<index_t I>
  void set_inverse(ikey<rank> key, ikey<inv_ranks[I]> value) {
    STORE_T::template set_inverse<I>(key, value);
  }
};
} // namespace dlm
