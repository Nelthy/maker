#pragma once
#include "ikey.h"
#include "tri_utils.h"
#include <array>
#include <cassert>

template<unsigned DIM, typename T>
class tri_array {
public:
  static constexpr unsigned data_size = tri_linearize(DIM - 1, DIM - 1, DIM) + 1;
  static constexpr unsigned dim = DIM;

  constexpr T &operator[](const dlm::ikey<2> &key) {
    auto r = key[0];
    auto c = key[1];
    assert(r < DIM && r <= c);
    return data[tri_linearize(r, c, DIM)];
  }

  constexpr const T &operator[](const dlm::ikey<2> &key) const {
    auto r = key[0];
    auto c = key[1];
    assert(r < DIM && r <= c);
    return data[tri_linearize(r, c, DIM)];
  }

  constexpr T &operator[](unsigned i) { return data[i]; }

  constexpr const T &operator[](unsigned i) const { return data[i]; }

  constexpr tri_array() = default;
  constexpr ~tri_array() = default;

  std::array<T, data_size> data{};

private:
};
