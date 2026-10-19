#pragma once
#include "ds_modifier.h"
#include <array>
#include "basic_types.h"

namespace dlm {
template<index_t SIZE, typename T>
class arr_ds : public std::array<T, SIZE> {
public:
  static constexpr bool is_comp = has_rank<T>;
  static constexpr rank_t rank = 1;

protected:
  friend class ds_modifier;

  void resize(index_t new_size) { assert(new_size == this->size()); }
  void swap(arr_ds &other, index_t a, index_t b) { std::swap(this->operator[](a), other[b]); };
  void move(arr_ds &other, index_t dst, index_t src) { this->operator[](dst) = std::move(other[src]); };
};

template<index_t SIZE>
class arr_size_ds {
public:
  static constexpr index_t rank = 1;
  static constexpr index_t size() { return SIZE; }

protected:
  friend class ds_modifier;

  void resize(index_t new_size) { assert(new_size == size()); }
  void swap(arr_size_ds &other, index_t a, index_t b) {};
  void move(arr_size_ds &other, index_t dst, index_t src) {};
};

} // namespace dlm
