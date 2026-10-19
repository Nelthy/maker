#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/color_decl.h"
#include "basic_color_data.h"
#include "data_set_tuple.h"
#include "ds_utils.h"
#include "maths.h"
#include "vec_ds.h"
#include <iostream>

namespace cp6 {
struct vbd_3d_qd:
dlm::dst<
  dlm::vec_ds<vec3f>,     // <0> q_old
  dlm::vec_ds<vec3f>,     // <1> q
  dlm::vec_ds<vec3f>,     // <2> q_dot
  dlm::vec_ds<vec3f>,     // <3> a_ext
  dlm::vec_ds<vec3f>,     // <4> y
  dlm::vec_ds<vec3f>,     // <5> f
  dlm::vec_ds<mat3f>,     // <6> H
  dlm::vec_ds<vec3f>,     // <7> mass
  dlm::vec_ds<vbd_flags>, // <8> immovable
  dlm::vec_ds<vec3f>,     // <9> q_dot_old
  dlm::vec_ds<vec3f>,     // <10> impulse_sum
  dlm::vec_ds<vec3f>      // <11> f_sum
> {
  DEFINE_DS_FIELD((0), q_old);
  DEFINE_DS_FIELD((1), q);
  DEFINE_DS_FIELD((2), q_dot);
  DEFINE_DS_FIELD((3), a_ext);
  DEFINE_DS_FIELD((4), y);
  DEFINE_DS_FIELD((5), f);
  DEFINE_DS_FIELD((6), H);
  DEFINE_DS_FIELD((7), mass);
  DEFINE_DS_FIELD((8), flags);
  DEFINE_DS_FIELD((9), q_dot_old);
  DEFINE_DS_FIELD((10), impulse);
  DEFINE_DS_FIELD((11), f_sum);

  vbd_3d_qd &qd() { return *this; }

  const vbd_3d_qd &qd() const { return *this; }

  bool start_step(float h);
  bool end_step(float h);
  bool init_derivs(float h);

  bool newton_step();
  bool newton_step(const basic_color_data &color);
};

SET_COLOR_DATA_T(vbd_3d_qd, basic_color_data);

inline vec2f transform_point(unsigned int q, const cp6::vbd_3d_qd &qt, const vec2f &point) {
  return transform(qt.q()[q], point);
}

inline vec2f inv_transform_point(unsigned int q, const cp6::vbd_3d_qd &qt, const vec2f &point) {
  return inv_transform(qt.q()[q], point);
}

inline vec2f transform_point_old(unsigned int q, const cp6::vbd_3d_qd &qt, const vec2f &point) {
  return transform(qt.q_old()[q], point);
}

inline vec2f inv_transform_point_old(unsigned int q, const cp6::vbd_3d_qd &qt, const vec2f &point) {
  return inv_transform(qt.q_old()[q], point);
}

inline vec2f translation(unsigned q, const cp6::vbd_3d_qd &qd) { return qd.q()[q].xy(); };
} // namespace cp6
