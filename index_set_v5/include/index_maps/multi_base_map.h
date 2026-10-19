#pragma once
#include "base_map.h"
#include "basic_types.h"
#include "ds_modifier.h"
#include "get.h"
#include <utility>

namespace dlm {
// STANDARD:
// STORE_T is expected to have the following functions exposed to base_map
//  static constexpr rank_t rank
//  static constexpr index_t map_num
//  static constexpr rank_t inv_ranks[map_num]
//  set_inverse<I>
//  get_inverse<I>
//  is_0_valid<I>
//  STORE_T must also be a valid data set

//////////////////////////////////////////////////////////////////////////////////////
// STORE_T: key storage type
template<typename STORE_T>
class multi_base_map: protected STORE_T {
public:
  static constexpr index_t map_num = STORE_T::map_num;
  static constexpr rank_t rank = STORE_T::rank;
  static constexpr auto inv_ranks = STORE_T::inv_ranks;

  using STORE_T::get_inverse;
  using STORE_T::is_0_valid;
  using STORE_T::size;
  decltype([]<size_t... Is>(std::index_sequence<Is...>) {
    return std::tuple<map_interface<inv_ranks[Is], rank> *...>{};
  }(std::make_index_sequence<map_num>{})) m_inverse_interfaces;

  // directly links two indices.
  template<index_t I>
  bool link(const ikey<rank> &a, const ikey<inv_ranks[I]> &b) {
    auto &inverse = *get<I>(m_inverse_interfaces);
    // cannot link already linked indices
    assert(!inverse.is_0_valid(b));
    assert(!STORE_T::template is_0_valid<I>(a));

    // both keys are not linked, so we can link them
    STORE_T::template set_inverse<I>(a, b);
    inverse.set_inverse(b, a);

    return true;
  }


  template<rank_t KEY_RANK>
  const auto &operator[](const ikey<KEY_RANK> &key) const {
    return STORE_T::operator[](key);
  }

  const auto &operator[](index_t key) const { return STORE_T::operator[](key); }

  const auto &get_store() { return static_cast<STORE_T &>(*this); }

protected:
  friend class ds_modifier;

  template<rank_t KEY_RANK>
  void null(const ikey<KEY_RANK> &key) {
    assert(key.is_valid());
    if constexpr (KEY_RANK == rank) {
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (assert(get<Is>(m_inverse_interfaces) && ...));
        (
          [&] {
            if (STORE_T::template is_0_valid<Is>(key)) {
              ikey<inv_ranks[Is]> inv_key = STORE_T::template get_inverse<Is>(key);
              STORE_T::template set_inverse<Is>(
                key,
                {}
              ); // nulling before unlinking stops circular dependencies
              get<Is>(m_inverse_interfaces)->unlink(inv_key);
            }
          }(),
          ...
        );
      }(std::make_index_sequence<map_num>{});
    } else {
      for (index_t i = 0; i < size(key); i++) {
        null(key.add_end(i));
      }
    }
  }

private:
  template<rank_t KEY_RANK>
  void set_inverse_key(ikey<KEY_RANK> key, ikey<KEY_RANK> new_inverse) {
    assert(key.is_valid());
    assert(new_inverse.is_valid());

    if constexpr (KEY_RANK == rank) {
      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (assert(get<Is>(m_inverse_interfaces) && ...));
        (
          [&] {
            if (STORE_T::template is_0_valid<Is>(key)) {
              get<Is>(m_inverse_interfaces)
                ->set_inverse(STORE_T::template get_inverse<Is>(key), new_inverse);
            }
          }(),
          ...
        );
      }(std::make_index_sequence<map_num>{});

    } else {
      for (index_t i = 0; i < size(key); i++) {
        set_inverse_key(key.add_end(i), new_inverse.add_end(i));
      }
    }
  }

  template<rank_t KEY_RANK>
  void move(const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
    set_inverse_key(src, dst);
    ds_modifier::move(*static_cast<STORE_T *>(this), dst, src);
  }

  template<rank_t KEY_RANK>
  void swap(const ikey<KEY_RANK> &a, const ikey<KEY_RANK> &b) {
    set_inverse_key(a, b);
    ds_modifier::swap(*static_cast<STORE_T *>(this), a, b);
    set_inverse_key(a, a);
  }

  void flush_connected() {
    [&]<size_t... Is>(std::index_sequence<Is...>) {
      (assert(get<Is>(m_inverse_interfaces) && ...));
      (get<Is>(m_inverse_interfaces)->m_clear_null_fn(get<Is>(m_inverse_interfaces)->m_root_perm),
       ...);
    }(std::make_index_sequence<map_num>{});
  }

  void clear_null() {
    // this flushes all of the connected maps
    flush_connected();
  }

  template<rank_t KEY_RANK>
  void clear(const ikey<KEY_RANK> &key) {
    static_assert(KEY_RANK < rank, "operation rank too high");
    for (index_t i = 0; i < size(key); i++) {
      null(key.add_end(i));
    }
    flush_connected();
    ds_modifier::clear(*static_cast<STORE_T *>(this), key);
  }
};

} // namespace dlm
