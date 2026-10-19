#pragma once
#include "maths.h"

inline vec2f transform_point(unsigned q, float *const &transform_data, const vec2f &point) {
  return rotate(point, transform_data[q]);
}

inline vec2f transform_point(unsigned q, vec2f *const &transform_data, const vec2f &point) {
  return point + transform_data[q];
}

inline vec2f transform_point(unsigned q, vec3f *const &transform_data, const vec2f &point) {
  return rotate(point, transform_data[q].z) + transform_data[q].xy();
}
