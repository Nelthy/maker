#pragma once
#include "basic_types.h"
#include "ds_modifier.h"
#include <vector>

namespace dlm {
template<typename T, typename ALLOC>
class vec_ds_alloc: protected std::vector<T, ALLOC> {
protected:
  friend class ds_modifier;
  using vec_t = std::vector<T, ALLOC>;
  using vec_t::clear;
  using vec_t::resize;

  void swap(vec_ds_alloc &other, index_t a, index_t b) { std::swap(operator[](a), other[b]); }

  void move(vec_ds_alloc &other, index_t dst, index_t src) {
    operator[](dst) = std::move(other[src]);
  }

  void copy(const vec_ds_alloc &src_ds, index_t dst, index_t src) { operator[](dst) = src_ds[src]; }

  template<typename T1, typename A>
  friend class vec_ds_alloc;

public:
  static constexpr bool is_comp = has_rank<T>;
  static constexpr rank_t rank = 1;
  using vec_t::at;
  using vec_t::operator[];
  using vec_t::assign;
  using vec_t::back;
  using vec_t::begin;
  using vec_t::capacity;
  using vec_t::cbegin;
  using vec_t::cend;
  using vec_t::crbegin;
  using vec_t::crend;
  using vec_t::data;
  using vec_t::empty;
  using vec_t::end;
  using vec_t::front;
  using vec_t::max_size;
  using vec_t::rbegin;
  using vec_t::rend;
  using vec_t::reserve;
  using vec_t::shrink_to_fit;

  index_t size(ikey<0> k = {}) const { return vec_t::size(); }

  const vec_t &get_base() const { return static_cast<const vec_t &>(*this); }
};

} // namespace dlm
