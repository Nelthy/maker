#include "avbd_basic/liquid/vbd_liquid_qd.h"

namespace cp6 {
void vbd_start_step(cp6::vbd_liquid_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    qt.q_old()[q] = qt.q()[q];
    qt.y()[q] = qt.q()[q] + qt.q_dot()[q] * h + qt.a_ext()[q] * h * h;

    qt.q()[q] = qt.y()[q];
  }
}

void vbd_end_step(cp6::vbd_liquid_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    if (qt.flags()[q].immovable) {
      continue;
    }
    qt.q_dot_old()[q] = qt.q_dot()[q];
    qt.q_dot()[q] = (qt.q()[q] - qt.q_old()[q]) / h;
    qt.q_dot()[q] *= 0.999;
    qt.a_ext()[q] = {0, 0};
  }
}

void init_derivs(cp6::vbd_liquid_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    if (qt.flags()[q].immovable) {
      continue;
    }
    float alpha = 1; //.001;//.0000000001;
    qt.f()[q] = (qt.q()[q] - qt.y()[q]);
    scale_left(qt.f()[q], qt.mass()[q] / (h * h));
    qt.f()[q] *= alpha;

    qt.H()[q] = identity<2>();
    scale_left(qt.H()[q], qt.mass()[q] / (h * h));
    qt.H()[q] *= alpha;
  }
};

} // namespace cp6
