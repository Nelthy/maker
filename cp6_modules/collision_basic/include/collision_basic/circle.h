#pragma once
#include <maths.h>

namespace cp6{
struct circle {
  vec2f pos;
  float radius = 50;
  float calculate_area()const { return pi_v * radius * radius; }
  float calculate_area_inertia()const { return (pi_v * radius * radius * radius * radius) / 4.0f; }
};
}
