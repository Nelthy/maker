#include "avbd_basic/vbd_2d_qd.h"

namespace cp6 {

bool vbd_2d_qd::start_step(float h) {
  for (unsigned _q = 0; _q < size(); _q++) {
    q_old()[_q] = q()[_q];
    y()[_q] = q()[_q] + q_dot()[_q] * h + a_ext()[_q] * h * h;

    const vec2f acc = (q_dot()[_q] - q_dot_old()[_q]) / h;
    const float ext_a_length = length(a_ext()[_q]);

    if (ext_a_length == 0 || flags()[_q].immovable) {
      q()[_q] = q()[_q] + q_dot()[_q] * h;
      continue;
    }

    const vec2f a_ext_hat = a_ext()[_q] / ext_a_length;

    const float proj_acc = dot(a_ext_hat, acc);

    float a;
    if (proj_acc > ext_a_length) {
      a = 1;
    } else if (proj_acc < 0) {
      a = 0;
    } else {
      a = proj_acc / ext_a_length;
    }

    q()[_q] = q()[_q] + q_dot()[_q] * h + a_ext()[_q] * a * h * h;
  }
  return true;
}

bool vbd_2d_qd::end_step(float h) {
  for (unsigned _q = 0; _q < size(); _q++) {
    if (flags()[_q].immovable) { continue; }
    q_dot_old()[_q] = q_dot()[_q];
    q_dot()[_q] = (q()[_q] - q_old()[_q]) / h;
    a_ext()[_q] = {0, 0};
  }
  return true;
}

bool vbd_2d_qd::init_derivs(float h) {
  for (unsigned _q = 0; _q < size(); _q++) {
    if (flags()[_q].immovable) { continue; }
    f()[_q] = (q()[_q] - y()[_q]);
    scale_left(f()[_q], mass()[_q] / (h * h));
    H()[_q] = identity<2>();
    scale_left(H()[_q], mass()[_q] / (h * h));
  }
  return true;
}

bool vbd_2d_qd::newton_step() {
  for (unsigned _q = 0; _q < size(); _q++) {
    if (!flags()[_q].immovable) { q()[_q] -= solve(H()[_q], f()[_q]) * .5; }
  }
  return true;
}

bool vbd_2d_qd::newton_step(const basic_color_data &color) {
  for (unsigned cq = 0; cq < color.cq_num; cq++) {
    unsigned _q = color.to_q[cq];
    if (!flags()[_q].immovable) { q()[_q] -= solve(H()[_q], f()[_q]) * .5; }
  }
  return true;
}
} // namespace cp6
