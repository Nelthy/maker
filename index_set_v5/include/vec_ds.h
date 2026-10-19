#pragma once
#include "basic_types.h"
#include "ds_utils.h"
#include <vector>

#define DEFINE_VEC_DS_DATA(vec_ds_perm, name)                                                      \
  auto *name() { return this->template get<UNPAREN(vec_ds_perm)>().data(); }                       \
  const auto *name() const { return this->template get<UNPAREN(vec_ds_perm)>().data(); }

namespace dlm {
template<typename T, rank_t RANK = 1>
class vec_ds: protected std::vector<vec_ds<T, RANK - 1>> {
protected:
  friend class ds_modifier;
  template<typename T1, rank_t R>
  friend class vec_ds;

  using vec_t = std::vector<vec_ds<T, RANK - 1>>;
  using vec_t::clear;
  using vec_t::resize;

  template<rank_t KEY_RANK = 1>
  void swap(vec_ds &other, ikey<KEY_RANK> a, ikey<KEY_RANK> b) {
    std::swap(get_element(*this, a), get_element(other, b));
  }

  template<rank_t KEY_RANK = 1>
  void move(vec_ds &other, ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    get_element(*this, dst) = std::move(get_element(other, src));
  }

  template<rank_t KEY_RANK = 1>
  void copy(const vec_ds &src_ds, ikey<KEY_RANK> dst, ikey<KEY_RANK> src) {
    get_element(*this, dst) = get_element(src_ds, src);
  }

  template<rank_t KEY_RANK>
  void resize(index_t new_size, ikey<KEY_RANK> key) {
    get_element(*this, key).resize(new_size);
  }

  template<rank_t KEY_RANK>
  void clear(ikey<KEY_RANK> key) {
    get_element(*this, key).clear();
  }

public:
  static constexpr bool is_comp = has_rank<T>;
  static constexpr rank_t rank = RANK;

  template<rank_t KEY_RANK>
  index_t size(ikey<KEY_RANK> key) const {
    return get_element(*this, key).size();
  }

  T &operator[](ikey<RANK> key) { return get_element(*this, key); }
  const T &operator[](ikey<RANK> key) const { return get_element(*this, key); }

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
  using vec_t::size;

  const vec_t &get_base() const { return static_cast<const vec_t &>(*this); }
};

template<typename T>
class vec_ds<T, 1>: protected std::vector<T> {
protected:
  friend class ds_modifier;
  using vec_t = std::vector<T>;
  using vec_t::clear;
  using vec_t::resize;

  void swap(vec_ds &other, index_t a, index_t b) { std::swap(operator[](a), other[b]); }

  void move(vec_ds &other, index_t dst, index_t src) { operator[](dst) = std::move(other[src]); }

  void copy(const vec_ds &src_ds, index_t dst, index_t src) { operator[](dst) = src_ds[src]; }

  template<typename T1, rank_t R>
  friend class vec_ds;

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

template<>
class vec_ds<bool, 1>: protected std::vector<bool> {
protected:
  friend class ds_modifier;
  using vec_t = std::vector<bool>;
  using vec_t::resize;

  void swap(vec_ds &other, index_t a, index_t b) {
    bool tmp = operator[](a);
    operator[](a) = other[b];
    other[b] = tmp;
  }

  void move(vec_ds &other, index_t dst, index_t src) { operator[](dst) = other[src]; }

  void copy(const vec_ds &other, index_t dst, index_t src) { operator[](dst) = other[src]; }

  template<typename T1, rank_t R>
  friend class vec_ds;

public:
  static constexpr bool is_comp = false;
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
  using vec_t::empty;
  using vec_t::end;
  using vec_t::front;
  using vec_t::max_size;
  using vec_t::rbegin;
  using vec_t::rend;
  using vec_t::reserve;
  using vec_t::shrink_to_fit;
  using vec_t::size;

  const vec_t &get_base() const { return static_cast<const vec_t &>(*this); }
};


} // namespace dlm
