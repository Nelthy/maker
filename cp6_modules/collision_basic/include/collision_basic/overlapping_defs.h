#pragma once
#include "aabb.h"
#include "collision_basic/circle_cd.h"
#include "collision_basic/poly_cd.h"

// must be included before basic_broadphase in order to not throw linker errors
namespace cp6 {
inline bool overlapping(const cp6::circle &a, const cp6::circle &b) {
  float dist_squared = length_squared(a.pos - b.pos);
  float radii_sum_squared = (a.radius + b.radius) * (a.radius + b.radius);
  if (dist_squared <= radii_sum_squared) {
    return true;
  }
  return false;
}

inline bool overlapping(const cp6::circle &c, const aabbf &other_bound) {
  aabbf circle_bound = {c.pos - vec2f{c.radius, c.radius}, c.pos + vec2f{c.radius, c.radius}};

  return aabbf::overlapping(circle_bound, other_bound);
}

inline bool overlapping(aabbf b1, aabbf b2) { return aabbf::overlapping(b1, b2); }
} // namespace cp6
