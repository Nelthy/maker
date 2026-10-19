#pragma once
#include "basic_types.h"
#include <concepts>
#include <cstring>
#include <type_traits>
#include <utility>

namespace dlm {
template<typename... Args>
concept ints = requires { requires((std::integral<Args> || std::is_enum_v<Args>) && ...); };

template<rank_t RANK>
struct index_array {
  template<ints... Is>
  constexpr index_array(Is... indices) : arr{static_cast<index_t>(indices)...} {};

  constexpr index_t &operator[](unsigned i) { return arr[i]; }

  constexpr const index_t &operator[](unsigned i) const { return arr[i]; }

  index_t arr[RANK];
};

template<rank_t RANK>
class ikey {
public:
  static constexpr rank_t key_rank = RANK;
  template<ints... Is>
    requires(sizeof...(Is) == RANK)
  constexpr ikey(Is... indices) : m_indices{static_cast<index_t>(indices)...} {};

  constexpr ikey() {
    if constexpr (RANK != 0) {
      std::memset(m_indices.arr, -1, sizeof(index_t) * RANK);
    }
  };

  constexpr ikey(const ikey &other) {
    if constexpr (RANK != 0) {
      m_indices = other.m_indices;
    }
  }

  constexpr ikey &operator=(const ikey &other) {
    if constexpr (RANK != 0) {
      m_indices = other.m_indices;
    }
    return *this;
  }

  constexpr ikey(ikey &&other) noexcept {
    if constexpr (RANK != 0) {
      m_indices = other.m_indices;
      std::memset(other.m_indices.arr, -1, sizeof(index_t) * RANK);
    }
  }

  constexpr ikey &operator=(ikey &&other) noexcept {
    if constexpr (RANK != 0) {
      m_indices = other.m_indices;
      std::memset(other.m_indices.arr, -1, sizeof(index_t) * RANK);
    }
    return *this;
  }

  template<rank_t... RANKS>
    requires((RANKS + ...) == RANK)
  constexpr ikey(const ikey<RANKS> &...keys) {
    if constexpr (RANK != 0) {
      constexpr auto ind = [] {
        index_array<sizeof...(RANKS)> end_ind{RANKS...};
        index_t cursor = 0;
        for (rank_t i = 0; i < sizeof...(RANKS); i++) {
          index_t curr_rank = end_ind[i];
          end_ind[i] = cursor;
          cursor += curr_rank;
        }
        return end_ind;
      }();

      [&]<size_t... Is>(std::index_sequence<Is...>) {
        (std::memcpy(&m_indices[ind[Is]], &keys, sizeof(index_t) * RANKS), ...);
      }(std::make_index_sequence<sizeof...(RANKS)>{});
    }
  }

  constexpr operator index_t() const
    requires(RANK == 1)
  {
    return m_indices[0];
  };

  constexpr index_t &operator[](index_t i) { return m_indices[i]; }

  constexpr const index_t &operator[](index_t i) const { return m_indices[i]; }

  index_t &end()
    requires(RANK > 0)
  {
    return m_indices[RANK - 1];
  }

  const index_t &end() const
    requires(RANK > 0)
  {
    return m_indices[RANK - 1];
  }

  bool is_0_valid() const {
    if constexpr (RANK == 0) {
      return true;
    } else {
      return m_indices[0] != -1;
    }
  }

  bool is_valid() const {
    if constexpr (RANK != 0) {
      for (index_t i = 0; i < RANK; i++) {
        if (m_indices[i] == -1) {
          return false;
        }
      }
      return true;
    } else {
      return true;
    }
  }

  template<rank_t AMOUNT = 1>
    requires(AMOUNT <= RANK)
  ikey<RANK - AMOUNT> cut_end() const {
    ikey<RANK - AMOUNT> end_key(true);
    std::memcpy(&end_key.m_indices, &m_indices, sizeof(index_t) * (RANK - AMOUNT));
    return end_key;
  }

  template<rank_t AMOUNT = 1>
    requires(AMOUNT <= RANK)
  ikey<RANK - AMOUNT> cut_beg() const {
    ikey<RANK - AMOUNT> end_key(true);
    std::memcpy(&end_key.m_indices, &m_indices[AMOUNT], sizeof(index_t) * (RANK - AMOUNT));
    return end_key;
  }

  template<rank_t _RANK>
  ikey<RANK + _RANK> add_end(const ikey<_RANK> &other) const {
    return ikey<RANK + _RANK>{*this, other};
  }

  template<rank_t _RANK>
  ikey<RANK + _RANK> add_beg(const ikey<_RANK> &other) const {
    return ikey<RANK + _RANK>{other, *this};
  }

  ikey<RANK + 1> add_end(index_t i) const { return ikey<RANK + 1>{*this, ikey<1>{i}}; }

  ikey<RANK + 1> add_beg(index_t i) const { return ikey<RANK + 1>{ikey<1>{i}, *this}; }

  template<rank_t NEW_RANK>
    requires(NEW_RANK <= RANK)
  ikey<NEW_RANK> shrink_to_beg() const {
    return cut_end<RANK - NEW_RANK>();
  }

  template<rank_t NEW_RANK>
    requires(NEW_RANK <= RANK)
  ikey<NEW_RANK> shrink_to_end() const {
    return cut_beg<RANK - NEW_RANK>();
  }

  std::conditional_t<(bool)RANK, index_array<RANK>, std::false_type> m_indices;

private:
  template<rank_t _RANK>
  friend class ikey;

  explicit ikey(bool no_init) {}
};

template<ints... Args>
ikey(Args...) -> ikey<sizeof...(Args)>;

template<rank_t... RANKS>
ikey(const ikey<RANKS> &...) -> ikey<(RANKS + ...)>;

template<rank_t RANK>
bool operator==(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; ++i) {
    if (a[i] != b[i]) {
      return false;
    }
  }
  return true;
}

template<rank_t RANK>
bool operator!=(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; i++) {
    if (a[i] != b[i]) {
      return true;
    }
  }
  return false;
}

template<rank_t RANK>
bool operator<(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; i++) {
    if (a[i] < b[i]) {
      return true;
    }
    if (a[i] > b[i]) {
      return false;
    }
  }
  return false; // they are equal
}

template<rank_t RANK>
bool operator>(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; i++) {
    if (a[i] > b[i]) {
      return true;
    }
    if (a[i] < b[i]) {
      return false;
    }
  }
  return false; // they are equal
}

template<rank_t RANK>
bool operator<=(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; i++) {
    if (a[i] < b[i]) {
      return true;
    }
    if (a[i] > b[i]) {
      return false;
    }
  }
  return true; // they are equal
}

template<rank_t RANK>
bool operator>=(const ikey<RANK> &a, const ikey<RANK> &b) {
  for (rank_t i = 0; i < RANK; i++) {
    if (a[i] > b[i]) {
      return true;
    }
    if (a[i] < b[i]) {
      return false;
    }
  }
  return true; // they are equal
}

inline bool operator==(const ikey<0> &a, const ikey<0> &b) { return true; }

inline bool operator!=(const ikey<0> &a, const ikey<0> &b) { return false; }

inline bool operator<(const ikey<0> &a, const ikey<0> &b) { return false; }

inline bool operator>(const ikey<0> &a, const ikey<0> &b) { return false; }

inline bool operator<=(const ikey<0> &a, const ikey<0> &b) { return true; }

inline bool operator>=(const ikey<0> &a, const ikey<0> &b) { return true; }

} // namespace dlm
