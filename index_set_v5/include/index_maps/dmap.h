#pragma once
#include "base_map.h"
#include "operation_fn_ptrs.h"

namespace dlm {
// dmap stands for dependent map.
// Every index is dependent on its inverse, so when the inverse unlinks from the index, the index
// gets removed
template<typename STORE_T>
class dmap: public base_map<STORE_T> {
public:
  using base = base_map<STORE_T>;
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr rank_t inv_rank = STORE_T::inv_rank;

  void unlink(ikey<rank> key) override { m_null_fn(base::m_root_perm, key); }

  bool is_0_valid(ikey<rank> key) override { return STORE_T::is_0_valid(key); }

  template<rank_t... Ps, typename IS>
  void init(IS &self_is, ckey<Ps...> self_perm, map_interface<inv_rank, rank> &inverse_ds) {
    base::init(self_is, self_perm, inverse_ds);
    m_null_fn = get_null_fn(self_is, self_perm);
  }
private:
  null_fn<rank> m_null_fn;
  void set_inverse(ikey<rank> key, ikey<inv_rank> value) override {
    STORE_T::set_inverse(key, value);
  }
};
} // namespace dlm
