#include "avbd_basic/vbd_2d_qd.h"

namespace cp6 {
void vbd_start_step(cp6::vbd_2d_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    qt.q_old()[q] = qt.q()[q];
    qt.y()[q] = qt.q()[q] + qt.q_dot()[q] * h + qt.a_ext()[q] * h * h;

    const vec2f acc = (qt.q_dot()[q] - qt.q_dot_old()[q]) / h;
    const float ext_a_length = length(qt.a_ext()[q]);

    if (ext_a_length == 0 || qt.flags()[q].immovable) {
      qt.q()[q] = qt.q()[q] + qt.q_dot()[q] * h;
      continue;
    }

    const vec2f a_ext_hat = qt.a_ext()[q] / ext_a_length;

    const float proj_acc = dot(a_ext_hat, acc);

    float a;
    if (proj_acc > ext_a_length) {
      a = 1;
    } else if (proj_acc < 0) {
      a = 0;
    } else {
      a = proj_acc / ext_a_length;
    }

    qt.q()[q] = qt.q()[q] + qt.q_dot()[q] * h + qt.a_ext()[q] * a * h * h;
  }
}

void vbd_end_step(cp6::vbd_2d_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    if (qt.flags()[q].immovable) {
      continue;
    }
    qt.q_dot_old()[q] = qt.q_dot()[q];
    qt.q_dot()[q] = (qt.q()[q] - qt.q_old()[q]) / h;
    qt.a_ext()[q] = {0, 0};
  }
}

void init_derivs(cp6::vbd_2d_qd &qt, float h) {
  for (unsigned q = 0; q < qt.size(); q++) {
    if (qt.flags()[q].immovable) {
      continue;
    }
    qt.f()[q] = (qt.q()[q] - qt.y()[q]);
    scale_left(qt.f()[q], qt.mass()[q] / (h * h));
    qt.H()[q] = identity<2>();
    scale_left(qt.H()[q], qt.mass()[q] / (h * h));
  }
};
} // namespace cp6
