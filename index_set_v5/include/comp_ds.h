#pragma once
#include "basic_types.h"
#include "ds_flags.h"
#include "ds_modifier.h"
#include "ds_utils.h"
#include <type_traits>
#include <utility>

namespace dlm {
// the ranks of the comp data sets need to be added together to get the rank of their composition.
template<typename T, rank_t CURR_RANK = 0>
constexpr rank_t comp_rank() {
  if constexpr (!is_comp<T>) {
    if constexpr (has_rank<T>) {
      return CURR_RANK + T::rank;
    } else {
      return CURR_RANK;
    }
  } else {
    static_assert(T::rank != 0);
    if constexpr (has_bracket<T, T::rank>) {
      return comp_rank<bracket_value_type<T, T::rank>, CURR_RANK + T::rank>();
    } else {
      assert(false);
      // static_assert(false, "comp data set didn't have an operator that gave the composed type");
    }
  }
}

// returns the data set holding the rank
template<typename T, rank_t RANK>
inline auto &chunky_get_element(T &t, const ikey<RANK> &key) {
  assert(key.is_valid());
  if constexpr (is_comp<T> && T::rank <= RANK) {
    static_assert(T::rank != 0);
    if constexpr (has_bracket<T, T::rank>) {
      return chunky_get_element(
        t[key.template shrink_to_beg<T::rank>()],
        key.template cut_beg<T::rank>()
      );
    } else {
      assert(false);
      return std::false_type{};
      // static_assert(false, "comp data set didn't have an operator that gave the composed type");
    }
  } else {
    return t;
  }
}

template<typename T, rank_t RANK>
inline const auto &chunky_get_element(const T &t, const ikey<RANK> &key) {
  assert(key.is_valid());
  if constexpr (is_comp<T> && T::rank <= RANK) {
    static_assert(T::rank != 0);
    static_assert(has_bracket<T, T::rank>);
    return chunky_get_element(
      t[key.template shrink_to_beg<T::rank>()],
      key.template cut_beg<T::rank>()
    );
  } else {
    return t;
  }
}

// This returns the difference between the rank chunky_get_element gave you vs what you asked for.
// Think of this as returning the chunked data set's local rank
template<typename T, rank_t RANK, rank_t ORG_RANK = RANK, rank_t CURR_RANK = 0>
constexpr rank_t chunk_diff() {
  if constexpr (is_comp<T> && T::rank <= RANK) {
    static_assert(T::rank != 0);
    static_assert(has_bracket<T, T::rank>);
    return chunk_diff<
      bracket_value_type<T, T::rank>,
      RANK - T::rank,
      ORG_RANK,
      CURR_RANK + T::rank
    >();
  } else {
    return ORG_RANK - CURR_RANK;
  }
}

template<typename T, rank_t RANK>
using chunk_type = std::remove_cvref_t<
  decltype(chunky_get_element(std::declval<T &>(), std::declval<ikey<RANK>>()))
>;

// A wrapper around a nested data set that provides the necessary operations for it to work with
// ds_modifier. To be supported, a data set must have the 'is_comp' property and a operator[] that
// gives the composed type.
template<is_comp T>
class comp_ds: public T {
public:
  static constexpr rank_t rank = comp_rank<T>();

  template<rank_t RANK = 0>
  index_t size(ikey<RANK> key = {}) const {
    static_assert(RANK < rank);
    if constexpr (chunk_diff<T, RANK>() == 0) {
      return ds_modifier::size(chunky_get_element(get_type(), key), ikey{});
    } else {
      return ds_modifier::size(
        chunky_get_element(get_type(), key),
        key.template shrink_to_end<chunk_diff<T, RANK>()>()
      );
    }
  }

  using T::operator[];

  auto &operator[](ikey<rank - chunk_type<T, rank>::rank> key) {
    assert(key.is_valid());
    return chunky_get_element(get_type(), key);
  }

  const auto &operator[](ikey<rank - chunk_type<T, rank>::rank> key) const {
    assert(key.is_valid());
    return chunky_get_element(get_type(), key);
  }

  template<bool B = false>
  auto &operator[](ikey<rank> key) {
    return get_element(*this, key);
  }

  template<bool B = false>
  const auto &operator[](ikey<rank> key) const {
    return get_element(*this, key);
  }

protected:
  friend class ds_modifier;

  inline T &get_type() { return *reinterpret_cast<T *>(this); }

  inline const T &get_type() const { return *reinterpret_cast<const T *>(this); }

  template<rank_t RANK>
  void resize(index_t new_size, const ikey<RANK> &key) {
    static_assert(RANK < rank);
    assert(is_key_valid(*this, key) && "dlm::comp_ds::resize invalid key");
    if constexpr (chunk_diff<T, RANK>() == 0) {
      ds_modifier::resize(chunky_get_element(get_type(), key), new_size, ikey{});
    } else {
      ds_modifier::resize(
        chunky_get_element(get_type(), key),
        new_size,
        key.template shrink_to_end<chunk_diff<T, RANK>()>()
      );
    }
  }

  template<rank_t RANK>
  void clear(const ikey<RANK> &key) {
    static_assert(RANK < rank);
    assert(is_key_valid(*this, key) && "dlm::comp_ds::clear invalid key");
    if constexpr (chunk_diff<T, RANK>() == 0) {
      ds_modifier::clear(chunky_get_element(get_type(), key), ikey{});
    } else {
      ds_modifier::clear(
        chunky_get_element(get_type(), key),
        key.template shrink_to_end<chunk_diff<T, RANK>()>()
      );
    }
  }

  template<rank_t RANK>
  void move(ikey<RANK> dst, ikey<RANK> src) {
    assert(is_key_valid(*this, dst) && "dlm::comp_ds::move invalid dst");
    assert(is_key_valid(*this, src) && "dlm::comp_ds::move invalid src");
    auto &dst_ds = chunky_get_element(get_type(), dst);
    auto &src_ds = chunky_get_element(get_type(), src);

    if constexpr (chunk_diff<T, RANK>() == 0) {
      dst_ds = std::move(src_ds);
    } else {
      ds_modifier::move(
        dst_ds,
        src_ds,
        dst.template shrink_to_end<chunk_diff<T, RANK>()>(),
        src.template shrink_to_end<chunk_diff<T, RANK>()>()
      );
    }
  }

  template<rank_t RANK>
  void swap(ikey<RANK> a, ikey<RANK> b) {
    assert(is_key_valid(*this, a) && "dlm::comp_ds::swap invalid a");
    assert(is_key_valid(*this, b) && "dlm::comp_ds::swap invalid b");

    auto &a_ds = chunky_get_element(get_type(), a);
    auto &b_ds = chunky_get_element(get_type(), b);

    if constexpr (chunk_diff<T, RANK>() == 0) {
      std::swap(a_ds, b_ds);
    } else {
      ds_modifier::swap(
        a_ds,
        b_ds,
        a.template shrink_to_end<chunk_diff<T, RANK>()>(),
        b.template shrink_to_end<chunk_diff<T, RANK>()>()
      );
    }
  };

  template<rank_t RANK>
  void null(ikey<RANK> key) {
    assert(is_key_valid(*this, key) && "dlm::comp_ds::null invalid key");

    static_assert(RANK != 0);
    if constexpr (chunk_diff<T, RANK>() == 0) {
      static constexpr rank_t chunk_key_rank = chunk_type<T, RANK - 1>::rank;
      ds_modifier::null(
        chunky_get_element(get_type(), key.cut_end()),
        key.template shrink_to_end<chunk_key_rank>()
      );
    } else {
      static constexpr rank_t chunk_key_rank = chunk_diff<T, RANK>();
      ds_modifier::null(
        chunky_get_element(get_type(), key.cut_end()),
        key.template shrink_to_end<chunk_key_rank>()
      );
    }
  }

  // clear_null is inherited from T
};

} // namespace dlm
