#pragma once
#include "multi_base_map.h"
#include "operation_fn_ptrs.h"

namespace dlm {
template<typename STORE_T>
class multi_imap;

template<rank_t RANK, rank_t INV_RANK, typename STORE_T, index_t I>
struct multi_imap_interface: map_interface<RANK, INV_RANK> {
private:
  template<typename _STORE_T>
  friend class multi_imap;

  void set_inverse(ikey<RANK> key, ikey<INV_RANK> value) override {
    m_map->template set_inverse<I>(key, value);
  };

  multi_imap<STORE_T> *m_map;

public:
  void unlink(ikey<RANK> key) override {
    // this store needs to be imap or imap here.
    m_map->template unlink<I>(key);
  };

  bool is_0_valid(ikey<RANK> key) override { return m_map->template is_0_valid<I>(key); };
};

// imap stands for independent map.
template<typename STORE_T>
class multi_imap: public multi_base_map<STORE_T> {
public:
  using base = multi_base_map<STORE_T>;
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr auto inv_ranks = STORE_T::inv_ranks;
  static constexpr index_t map_num = STORE_T::map_num;

  template<index_t I>
  auto &get_interface() {
    return get<I>(m_interfaces);
  }

  multi_imap() {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      ((get_interface<Is>().m_map = this), ...);
    }(std::make_index_sequence<map_num>{});
  }

  template<index_t I>
  void unlink(ikey<rank> key) {
    base::null(key);
  }

  template<index_t I>
  bool is_0_valid(ikey<rank> key) {
    return STORE_T::template is_0_valid<I>(key);
  }

  template<index_t I, rank_t... Ps, typename IS>
  void init(IS &self_is, ckey<Ps...> self_perm, map_interface<inv_ranks[I], rank> &inverse_ds) {
    get_interface<I>().init(self_is, self_perm, inverse_ds);
    get<I>(base::m_inverse_interfaces) = &inverse_ds;
  }

private:
  template<rank_t RANK, rank_t INV_RANK, typename _STORE_T, index_t I>
  friend struct multi_imap_interface;

  decltype([]<size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple<multi_imap_interface<rank, inv_ranks[Is], STORE_T, Is>...>{};
  }(std::make_index_sequence<map_num>{})) m_interfaces;

  template<index_t I>
  void set_inverse(ikey<rank> key, ikey<inv_ranks[I]> value) {
    STORE_T::template set_inverse<I>(key, value);
  }
};
} // namespace dlm
