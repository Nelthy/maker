#pragma once
#include "ikey.h"
#include "receiver_modifier.h"
#include <cstddef>

namespace dlm {
template<typename T, rank_t RANK = 1>
class pointer_relay_ds {
public:
  static constexpr rank_t rank = RANK;

  void init(T &t) { ptr = &t; }

private:
  friend class ds_modifier;

  template<rank_t KEY_RANK>
  void resize(unsigned new_size, ikey<KEY_RANK> key) {
    assert(ptr);
    receiver_modifier::on_resize(*ptr, new_size, key);
  }

  template<rank_t KEY_RANK>
  void swap(ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    assert(ptr);
    receiver_modifier::on_swap(*ptr, a, b);
  }

  template<rank_t KEY_RANK>
  void move(pointer_relay_ds &other, ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    assert(ptr);
    receiver_modifier::on_move(*ptr, dst, src);
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    assert(ptr);
    receiver_modifier::on_null(*ptr, key);
  }

  void clear_null() {
    assert(ptr);
    receiver_modifier::on_clear_null(*ptr);
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    assert(ptr);
    receiver_modifier::on_clear(*ptr, key);
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    assert(ptr);
    receiver_modifier::on_merge(*ptr, dst, src);
  }



  T *ptr;
};
} // namespace dlm
