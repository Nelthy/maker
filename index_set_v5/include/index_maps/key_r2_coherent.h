#pragma once
#include "ds_modifier.h"
#include "ikey.h"
#include <array>
#include <bitset>
#include <cassert>
#include <cstring>
#include <vector>

namespace dlm {
// Stores all the keys spanning two ranks in a single array
template<rank_t R1_SIZE, rank_t INV_RANK>
class key_r2_coherent {
public:
  static constexpr rank_t rank = 2;
  static constexpr rank_t inv_rank = INV_RANK;

  ikey<inv_rank> get_inverse(const ikey<rank> &key) const { return operator[](key); }

  bool is_0_valid(ikey<rank> key) const { return operator[](key).is_0_valid(); }

  const auto &operator[](const ikey<rank> &key) const {
    assert(key[0] < size());
    assert(key[1] < size(key.cut_end()));
    return rank2[rank2_start[key[0]] + key[1]];
  }

  auto &operator[](const ikey<rank> &key) {
    assert(key[0] < size());
    assert(key[1] < size(key.cut_end()));

    return rank2[rank2_start[key[0]] + key[1]];
  }

  template<rank_t KEY_DEPTH = 0>
  index_t size(const ikey<KEY_DEPTH> &key = {}) const {
    assert(key.is_valid());
    if constexpr (KEY_DEPTH == 0) {
      return R1_SIZE;
    } else {
      assert(key[0] < R1_SIZE);
      if (key[0] < R1_SIZE - 1) {
        return rank2_start[key[0] + 1] - rank2_start[key[0]];
      } else {
        return rank2.size() - rank2_start[key[0]];
      }
    }
  }

  key_r2_coherent() {
    for (index_t i = 0; i < R1_SIZE; i++) {
      rank2_start[i] = 0;
    }
  }

  key_r2_coherent(key_r2_coherent &&other) noexcept {
    rank2_start = other.rank2_start;
    for (index_t i = 0; i < R1_SIZE; i++) {
      other.rank2_start[i] = 0;
    }
    rank2.swap(other.rank2);
  }

  key_r2_coherent &operator=(key_r2_coherent &&other) noexcept {
    rank2_start = other.rank2_start;
    for (index_t i = 0; i < R1_SIZE; i++) {
      other.rank2_start[i] = 0;
    }
    rank2.swap(other.rank2);

    return *this;
  }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) { operator[](key) = value; };

  void swap_keys(std::vector<ikey<inv_rank>> &src, ikey<1> dst) {
    assert(false); // not tested
    this->resize(src.size(), dst);
    std::memcpy(
      &this->operator[](dst.add_end(0)),
      src.data(),
      src.size() * sizeof(ikey<inv_rank>) * src.size()
    );
  };

protected:
  friend class ds_modifier;

  template<rank_t KEY_RANK = 1>
  void swap(key_r2_coherent &other, const ikey<KEY_RANK> &a, const ikey<KEY_RANK> &b) {
    if constexpr (KEY_RANK == 1) {
      assert(false);
    } else if constexpr (KEY_RANK == 2) {
      std::swap(this->operator[](a), other.operator[](b));
    } else {
      assert(false);
      //static_assert(false);
    }
  }

  template<rank_t KEY_RANK = 1>
  void move(key_r2_coherent &other, const ikey<KEY_RANK> &dst, const ikey<KEY_RANK> &src) {
    if constexpr (KEY_RANK == 1) {
      assert(false);
    } else if constexpr (KEY_RANK == 2) {
      this->operator[](dst) = std::move(other.operator[](src));
    } else {
      assert(false);
      //static_assert(false);
    }
  }

  template<rank_t KEY_RANK>
  void resize(index_t new_size, ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == 0) {
      assert(new_size == R1_SIZE);
    } else if constexpr (KEY_RANK == 1) {
      index_t old_size = size(key);
      int size_diff = new_size - old_size;

      if (size_diff > 0) {
        // resize first so we have room to move
        index_t r2_old_size = rank2.size();
        rank2.resize(rank2.size() + size_diff);
        index_t r2_old_end = rank2_start[key[0]] + old_size;

        std::memmove(
          &rank2.data()[r2_old_end + size_diff][0],
          &rank2.data()[r2_old_end][0],
          (r2_old_size - r2_old_end) * sizeof(ikey<INV_RANK>)
        );

        // nulling out the newly created keys
        for (index_t i = r2_old_end; i < r2_old_end + size_diff; i++) {
          rank2[i] = {};
        }
      } else {
        index_t r2_old_end = rank2_start[key[0]] + old_size;
        std::memmove(
          &rank2.data()[r2_old_end + size_diff][0],
          &rank2.data()[r2_old_end][0],
          (rank2.size() - r2_old_end) * sizeof(ikey<INV_RANK>)
        );
        // resize second so we don't move from out of bounds
        rank2.resize(rank2.size() + size_diff);
      }

      for (index_t i = key[0] + 1; i < R1_SIZE; i++) {
        rank2_start[i] += size_diff;
      }
      // if (new_size == 0) {
      //   is_0_size[key[0]] = 1;
      // } else {
      //   is_0_size[key[0]] = 0;
      // }
    } else {
      assert(false);
      //static_assert(false);
    }
  }

  // std::bitset<R1_SIZE> is_0_size;
  std::array<index_t, R1_SIZE> rank2_start;
  std::vector<ikey<INV_RANK>> rank2;
};
} // namespace dlm
