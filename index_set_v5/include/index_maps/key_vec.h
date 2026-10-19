#pragma once
#include "vec_ds.h"

namespace dlm {
template<rank_t RANK, rank_t INV_RANK>
class key_vec: public vec_ds<ikey<INV_RANK>, RANK> {
public:
  using vec_t = vec_ds<ikey<INV_RANK>, RANK>;

  static constexpr rank_t rank = RANK;
  static constexpr rank_t inv_rank = INV_RANK;

  ikey<inv_rank> get_inverse(ikey<rank> key) const { return vec_t::operator[](key); }

  bool is_0_valid(ikey<rank> key) const { return vec_t::operator[](key).is_0_valid(); }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) { vec_t::operator[](key) = value; };

  void swap_keys(std::vector<ikey<inv_rank>> &src, const ikey<rank - 1> &dst) {
    get_element(*this, dst).swap(src);
  };
};
} // namespace dlm
