#pragma once
#include "base_map.h"

namespace dlm {
// a map where all indices are independent of their inverses
template<typename STORE_T>
class imap: public base_map<STORE_T> {
public:
  using base = base_map<STORE_T>;
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr rank_t inv_rank = STORE_T::inv_rank;
  
  void unlink(ikey<rank> key) override { base::null(key); }

  bool is_0_valid(ikey<rank> key) override { return STORE_T::is_0_valid(key); }

private:
  void set_inverse(ikey<rank> key, ikey<inv_rank> value) override { STORE_T::set_inverse(key, value); }
};
} // namespace dlm
