#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/color_decl.h"
#include "basic_color_data.h"
#include "data_set_tuple.h"
#include "maths.h"
#include "vec_ds.h"

namespace cp6 {
struct vbd_2d_qd:
dlm::dst<
  dlm::vec_ds<vec2f>,     // <0> q_old
  dlm::vec_ds<vec2f>,     // <1> q
  dlm::vec_ds<vec2f>,     // <2> q_dot
  dlm::vec_ds<vec2f>,     // <3> a_ext
  dlm::vec_ds<vec2f>,     // <4> y
  dlm::vec_ds<vec2f>,     // <5> f
  dlm::vec_ds<mat2f>,     // <6> H
  dlm::vec_ds<vec2f>,     // <7> mass
  dlm::vec_ds<vbd_flags>, // <8> immovable
  dlm::vec_ds<vec2f>      // <9> q_dot_old
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

  vbd_2d_qd &qd() { return *this; }

  const vbd_2d_qd &qd() const { return *this; }

  bool start_step(float h);
  bool end_step(float h);
  bool init_derivs(float h);

  bool newton_step();
  bool newton_step(const basic_color_data &color);
};

SET_COLOR_DATA_T(vbd_2d_qd, basic_color_data);

inline void newton_step(cp6::vbd_2d_qd &qt) {
  for (unsigned q = 0; q < qt.size(); q++) {
    if (!qt.flags()[q].immovable) {
      qt.q()[q] -= solve(qt.H()[q], qt.f()[q]) * .5;
    }
  }
}

inline void newton_step(cp6::vbd_2d_qd &qt, const basic_color_data &color) {
  for (unsigned cq = 0; cq < color.cq_num; cq++) {
    unsigned q = color.to_q[cq];
    if (!qt.flags()[q].immovable) {
      qt.q()[q] -= solve(qt.H()[q], qt.f()[q]) * .5;
    }
  }
}

void vbd_start_step(cp6::vbd_2d_qd &qt, float h);
void vbd_end_step(cp6::vbd_2d_qd &qt, float h);
void init_derivs(cp6::vbd_2d_qd &qt, float h);

inline vec2f transform_point(
  unsigned int q,
  const cp6::vbd_2d_qd &qt,
  const vec2f &point
) {
  return point + qt.q()[q];
}

inline vec2f inv_transform_point(
  unsigned int q,
  const cp6::vbd_2d_qd &qt,
  const vec2f &point
) {
  return point - qt.q()[q];
}

inline vec2f inv_transform_point_old(
  unsigned int q,
  const cp6::vbd_2d_qd &qt,
  const vec2f &point
) {
  return point - qt.q_old()[q];
}

inline vec2f transform_point_old(
  unsigned int q,
  const cp6::vbd_2d_qd &qt,
  const vec2f &point
) {
  return point + qt.q_old()[q];
}

inline vec2f translation(unsigned q, const cp6::vbd_2d_qd &qd) {
  return qd.q()[q];
};
} // namespace cp6
