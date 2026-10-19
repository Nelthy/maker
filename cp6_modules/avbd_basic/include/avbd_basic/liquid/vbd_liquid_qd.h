#pragma once
#include "avbd_basic/vbd_2d_qd.h"
#include "avbd/avbd_flags.h"
#include "ds_utils.h"
#include "flat_data_set_tuple.h"
#include <vector>

namespace cp6 {

struct liquid_vars {
  float lambda = 0;
  float k = 100;

  float C = 0;
  std::vector<unsigned> adj;
};

struct vbd_liquid_qd: flag_vbd_liquid, dlm::fdst<cp6::vbd_2d_qd, dlm::vec_ds<liquid_vars>> {
  DEFINE_DS_FIELD((1), liquid_data);

  vbd_liquid_qd &qd() { return *this; }

  const vbd_liquid_qd &qd() const { return *this; }
};

inline void newton_step(cp6::vbd_liquid_qd &qt, unsigned q) {
  if (!qt.flags()[q].immovable) {
    if (qt.f()[q] == vec2f{0, 0}) {
      return;
    }

    qt.q()[q] -= solve(qt.H()[q], qt.f()[q]) * 0.5;
  }
}

void vbd_start_step(cp6::vbd_liquid_qd &qt, float h);
void vbd_end_step(cp6::vbd_liquid_qd &qt, float h);
void init_derivs(cp6::vbd_liquid_qd &qt, float h);

inline vec2f transform_point(unsigned int q, const cp6::vbd_liquid_qd &qt, const vec2f &point) {
  return point + qt.q()[q];
}

inline vec2f inv_transform_point(unsigned int q, const cp6::vbd_liquid_qd &qt, const vec2f &point) {
  return point - qt.q()[q];
}

inline vec2f translation(unsigned q, const cp6::vbd_liquid_qd &qd) { return qd.q()[q]; };
} // namespace cp6
