#pragma once
#include <iostream>
#include <print_functions.h>

namespace dlm {
template<std::size_t N>
struct fixed_string {
  char value[N];

  constexpr fixed_string(const char (&str)[N]) {
    for (std::size_t i = 0; i < N; ++i)
      value[i] = str[i];
  }

  constexpr const char *data() const { return value; }
};

template<rank_t RANK, fixed_string NAME>
class print_ds {
public:
  static constexpr rank_t rank = RANK;

  bool enabled = true;

private:
  friend class ds_modifier;

  template<rank_t KEY_RANK>
  void resize(index_t new_size, const ikey<KEY_RANK> &key) {
    if constexpr (KEY_RANK == rank - 1) {
      if (enabled) std::cout << NAME.value << " resize: " << key << " to " << new_size << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void clear(const ikey<KEY_RANK> &key) {
    if constexpr (KEY_RANK == rank - 1) {
      if (enabled) std::cout << NAME.value << " clearing: " << key << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void move(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      if (enabled) std::cout << NAME.value << " move: " << dst << " <- " << src << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void move(print_ds &other, ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank) {
      if (enabled) std::cout << NAME.value << " other move: " << dst << " <- " << src << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void swap(ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    if constexpr (KEY_RANK == rank) {
      if (enabled) std::cout << NAME.value << " swap: " << a << " <-> " << b << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void swap(print_ds &other, ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    if constexpr (KEY_RANK == rank) {
      if (enabled) std::cout << NAME.value << " other swap: " << a << " <-> " << b << std::endl;
    }
  }

  template<rank_t KEY_RANK>
  void null(ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == rank) {
      if (enabled) std::cout << NAME.value << " nulling: " << key << std::endl;
    }
  }

  void clear_null() {
    if (enabled) std::cout << NAME.value << " clear_null " << std::endl;
  }

  template<rank_t KEY_RANK>
  void merge(ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    if constexpr (KEY_RANK == rank - 1) {
      if (enabled) std::cout << NAME.value << " merge: " << dst << " <- " << src << std::endl;
    }
  }
};
} // namespace dlm
