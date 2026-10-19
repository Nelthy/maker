#pragma once
#include "vec_ds_alloc.h"

namespace dlm {
template<rank_t INV_RANK, typename ALLOC>
class key_vec_alloc: public vec_ds_alloc<dlm::ikey<INV_RANK>, ALLOC> {
public:
  using vec_t = vec_ds_alloc<ikey<INV_RANK>, ALLOC>;

  static constexpr rank_t rank = 1;
  static constexpr rank_t inv_rank = INV_RANK;

  ikey<inv_rank> get_inverse(ikey<rank> key) const { return vec_t::operator[](key); }

  bool is_0_valid(ikey<rank> key) const { return vec_t::operator[](key).is_0_valid(); }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) { vec_t::operator[](key) = value; };

  void swap_keys(std::vector<ikey<inv_rank>> &src, const ikey<rank - 1> &dst) {
    get_element(*this, dst).swap(src);
  };
};
} // namespace dlm
