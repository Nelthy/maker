#pragma once
#include "merge_keys.h"

namespace dlm {

// merges the input key (k1) into the stored inverse key (k2)
template<typename STORE_T, rank_t... IND>
class merge_key_wrap: public STORE_T {

public:
  static constexpr index_t rank = STORE_T::rank;
  static constexpr auto ind = ckey<IND...>{};
  static constexpr index_t inv_rank = decltype(merge_keys(ikey<rank>{}, ikey<STORE_T::inv_rank>{}, ind))::key_rank;

  ikey<inv_rank> get_inverse(ikey<rank> key) const {
    assert(key.is_valid());
    auto compact_key = STORE_T::get_inverse(key);
    return uncompress_key(key, compact_key);
  }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) {
    assert(key.is_valid());
    ikey<STORE_T::inv_rank> compact_key = unmerge_key2<rank, STORE_T::inv_rank>(value, ind);
    STORE_T::set_inverse(key, compact_key);
  }

  static inline ikey<inv_rank> uncompress_key(
    ikey<STORE_T::rank> key,
    ikey<STORE_T::inv_rank> compact_inv_key
  ) {
    if (!compact_inv_key.is_0_valid()) {
      // for performance reasons, the base map only checks the key[0] for validity
      // so when partial key replace 0 with a valid value every time we run into problems.
      // To fix, check if the compress's 0 is valid and if its not,
      // return an key with a invalid 0.
      return ikey<inv_rank>{}; // fully null key
    } else {
      ikey<inv_rank> full_key = merge_keys(key, compact_inv_key, ind);
      return full_key;
    }
  }
};
} // namespace dlm
