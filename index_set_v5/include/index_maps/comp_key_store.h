#pragma once
#include "comp_ds.h"
#include "ds_flags.h"
#include "ds_utils.h"
#include <vector>

namespace dlm {

template<is_comp T>
class comp_key_store: public comp_ds<T> {
public:
  static constexpr rank_t rank = comp_rank<T>();
  static constexpr rank_t inv_rank = chunk_type<T, rank>::inv_rank;
  using comp_ds<T>::size;

  ikey<inv_rank> get_inverse(ikey<rank> key) const {
    static constexpr rank_t base_rank = rank - chunk_type<T, rank>::rank;
    return comp_ds<T>::operator[](key.template shrink_to_beg<base_rank>())
      .get_inverse(key.template cut_beg<base_rank>());
  }

  bool is_0_valid(ikey<rank> key) const {
    static constexpr rank_t base_rank = rank - chunk_type<T, rank>::rank;
    assert(is_key_valid(*this, key));
    return comp_ds<T>::operator[](key.template shrink_to_beg<base_rank>())
      .is_0_valid(key.template cut_beg<base_rank>());
  }

  using comp_ds<T>::operator[];

  ikey<inv_rank> operator[](ikey<rank> key) { return get_inverse(key); }

  void swap_keys(std::vector<ikey<inv_rank>> &src, ikey<rank - 1> dst) {
    static constexpr rank_t base_rank = rank - chunk_type<T, rank>::rank;
    return comp_ds<T>::operator[](dst.template shrink_to_beg<base_rank>())
      .swap_keys(src, dst.template cut_beg<base_rank>());
  }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) {
    assert(key.is_valid());
    static constexpr rank_t base_rank = rank - chunk_type<T, rank>::rank;
    comp_ds<T>::operator[](key.template shrink_to_beg<base_rank>())
      .set_inverse(key.template cut_beg<base_rank>(), value);
  }
};
} // namespace dlm
