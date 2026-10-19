#pragma once
#include "arr_ds.h"
#include "ikey.h"
#include <cstring>
#include <vector>

namespace dlm {
template<index_t SIZE, rank_t INV_RANK>
class key_fixed: public arr_ds<SIZE, ikey<INV_RANK>> {
public:
  using fixed_t = arr_ds<SIZE, ikey<INV_RANK>>;
  static constexpr rank_t rank = 1;
  static constexpr rank_t inv_rank = INV_RANK;

  ikey<inv_rank> get_inverse(index_t i) const { return fixed_t::operator[](i); }

  bool is_0_valid(index_t i) const { return fixed_t::operator[](i).is_0_valid(); }

  void set_inverse(index_t i, ikey<inv_rank> value) { fixed_t::operator[](i) = value; };

  void swap_keys(std::vector<ikey<inv_rank>> &src, const ikey<0> &dst) {
    assert(src.size() == SIZE);
    std::memcpy(this->data(), src.data(), sizeof(ikey<inv_rank>) * SIZE);
  };
};

} // namespace dlm
