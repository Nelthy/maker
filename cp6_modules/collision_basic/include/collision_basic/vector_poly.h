#pragma once
#include "maths.h"
#include <vector>

struct vector_poly {
  vector_poly(const std::vector<vec2f> &base_vertices);
  vector_poly() {};
  std::vector<vec2f> vertices;
  float calculate_area() const;
  float calculate_area_inertia() const;
  vec2f calculate_centroid() const;

  static vector_poly square_shape(float scale_x = 1, float scale_y = 1);
  static vector_poly pentagon_shape(float scale_x = 1, float scale_y = 1);
  static vector_poly hexagon_shape(float scale_x = 1, float scale_y = 1);
};
