#pragma once
#include "data_set_tuple.h"
#include "ikey.h"
#include "vec_ds.h"
#include <array>
#include <utility>
#include <vector>

namespace dlm {
template<rank_t... DIST>
class split_key_vec: public dst<vec_ds<ikey<DIST>>...> {
public:
  static constexpr rank_t rank = 1;
  static constexpr rank_t inv_rank = (DIST + ...);
  static constexpr auto dist_arr = std::array{DIST...};
  static constexpr auto dist_to_inv_rank = []() {
    std::array<rank_t, sizeof...(DIST)> return_arr;
    rank_t cursor = 0;
    for (index_t d = 0; d < return_arr.size(); d++) {
      return_arr[d] = cursor;
      cursor += dist_arr[d];
    }
    return return_arr;
  }();

  ikey<inv_rank> get_inverse(index_t key) const {
    ikey<inv_rank> return_key;
    [&]<size_t... D>(std::index_sequence<D...>) {
      (
        [&] {
          for (index_t d_i = 0; d_i < dist_arr[D]; d_i++) {
            (return_key[dist_to_inv_rank[D] + d_i] = this->template get<D>()[key][d_i]);
          }
        }(),
        ...
      );
    }(std::make_index_sequence<sizeof...(DIST)>{});
    return return_key;
  }

  bool is_0_valid(index_t key) const { return this->template get<0>()[key].is_0_valid(); }

  void set_inverse(index_t key, ikey<inv_rank> value) {
    [&]<size_t... D>(std::index_sequence<D...>) {
      (
        [&] {
          for (index_t d_i = 0; d_i < dist_arr[D]; d_i++) {
            this->template get<D>()[key][d_i] = value[dist_to_inv_rank[D] + d_i];
          }
        }(),
        ...
      );
    }(std::make_index_sequence<sizeof...(DIST)>{});
  };

  void swap_keys(std::vector<ikey<inv_rank>> &src, const ikey<rank - 1> &dst) {
    this->resize(src.size());
    for (index_t k = 0; k < src.size(); k++) {
      set_inverse(k, src[k]);
    }
  };
};
} // namespace dlm
