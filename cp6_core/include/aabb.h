#pragma once
#include "maths.h"
#include <algorithm> //only for min/max

struct aabbf {
  vec2f min;//{0, 0};
  vec2f max;//{0, 0};

  static aabbf encompassing(const aabbf &a, const aabbf &b) {
    return aabbf{
      {std::min(a.min.x, b.min.x), std::min(a.min.y, b.min.y)},
      {std::max(a.max.x, b.max.x), std::max(a.max.y, b.max.y)}
    };
  };

  static bool overlapping(const aabbf &a, const aabbf &b) {
    if ((a.min.x <= b.max.x) && (a.max.x >= b.min.x) && (a.min.y <= b.max.y) &&
        (a.max.y >= b.min.y)) {
      return true;
    }
    return false;
  }

  vec2f get_center() const { return min + ((max - min) / 2.f); }

  void encompass(const vec2f &v) {
    min.x = std::min(v.x, min.x);
    min.y = std::min(v.y, min.y);
    max.x = std::max(v.x, max.x);
    max.y = std::max(v.y, max.y);
  }

  void invalidiate() {
    max.x = 0;
    min.x = 1;
  }

  bool is_valid() { return (min.x < max.x) && (min.y < max.y); }
};

template<typename T2>
T2 &operator<<(T2 &os, const aabbf &p) {
  os << "{" << p.min << "," << p.max << "}";
  return os;
}
