#pragma once
#include "circle.h"
#include "vec_ds.h"
#include <type_traits>

// user defined function
template<typename TD>
vec2f transform_point(unsigned q, const TD &transform_data, const vec2f &point);

namespace cp6 {
struct circle_cd: dlm::vec_ds<circle> {
  // collider data
  circle_cd &cd() { return *this; }

  const circle_cd &cd() const { return *this; }

  // bounds data
  circle_cd &bd() { return *this; }

  const circle_cd &bd() const { return *this; }

  // helper functions
  void calculate_constants(unsigned q, float *area, float *area_inertia = nullptr) const;
};
} // namespace cp6

// could be a member function, but the aesthetics of this is better :P
void cd_transform_start(cp6::circle_cd &cd);

template<typename TD>
void cd_transform(cp6::circle_cd &cd, const TD &td) {
  static_assert(!std::is_same_v<TD, cp6::circle_cd>);
  for (unsigned c = 0; c < cd.size(); c++) {
    cd[c].pos = transform_point(c, td, cd[c].pos);
  }
}

template<typename TD>
void cd_ind_transform(cp6::circle_cd &cd, const TD &td, const unsigned *ind) {
  for (unsigned c = 0; c < cd.size(); c++) {
    unsigned qt_c = ind[c];
    cd[c].pos = transform_point(qt_c, td, cd[c].pos);
  }
}

inline void cd_transform_end(cp6::circle_cd &cd) {};
