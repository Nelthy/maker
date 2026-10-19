#include "collision_basic/circle_cd.h"
#include "aabb.h"
#include <cstddef>

void cp6::circle_cd::calculate_constants(unsigned q, float *area, float *area_inertia) const {
  if (area != nullptr) {
    *area = operator[](q).calculate_area();
  }
  if (area_inertia != nullptr) {
    *area_inertia = operator[](q).calculate_area_inertia();
  }
}

void cd_transform_start(cp6::circle_cd &cd) {
  for (unsigned i = 0; i < cd.size(); i++) {
    cd[i] = {0, 0};
  }
}
