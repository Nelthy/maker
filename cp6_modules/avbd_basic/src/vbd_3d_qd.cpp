#include "avbd_basic/vbd_3d_qd.h"

namespace cp6 {
bool vbd_3d_qd::start_step(float h) {
  for (unsigned qi = 0; qi < size(); qi++) {
    f_sum()[qi] = {0, 0, 0};
    q_old()[qi] = q()[qi];
    y()[qi] = q()[qi] + q_dot()[qi] * h + a_ext()[qi] * h * h;

    const vec3f acc = (q_dot()[qi] - q_dot_old()[qi]) / h;
    const float ext_a_length = length(a_ext()[qi]);
    if (ext_a_length == 0 || flags()[qi].immovable) {
      q()[qi] = q()[qi] + q_dot()[qi] * h;
      continue;
    }
    const vec3f a_ext_hat = a_ext()[qi] / ext_a_length;
    const float proj_acc = dot(a_ext_hat, acc);

    float a;
    if (proj_acc > ext_a_length) {
      a = 1;
    } else if (proj_acc < 0) {
      a = 0;
    } else {
      a = proj_acc / ext_a_length;
    }
    a = 1; // no dynamic bullshit. It makes the stack oscillate
    q()[qi] = q()[qi] + q_dot()[qi] * h + a_ext()[qi] * a * h * h;
  }
  return true;
}

bool vbd_3d_qd::end_step(float h) {
  for (unsigned qi = 0; qi < size(); qi++) {
    if (flags()[qi].immovable) {
      continue;
    }
    q_dot_old()[qi] = q_dot()[qi];
    q_dot()[qi] = ((q()[qi] - q_old()[qi]) / h) + impulse()[qi];
    a_ext()[qi] = {0, 0, 0};
    impulse()[qi] = {0, 0, 0};
  }
  return true;
}

bool vbd_3d_qd::init_derivs(float h) {
  for (unsigned qi = 0; qi < size(); qi++) {
    if (flags()[qi].immovable) {
      continue;
    }
    f()[qi] = (q()[qi] - y()[qi]);
    scale_left(f()[qi], mass()[qi] / (h * h));
    H()[qi] = identity<3>();
    scale_left(H()[qi], mass()[qi] / (h * h));
  }
  return true;
};

bool vbd_3d_qd::newton_step() {
  for (unsigned qi = 0; qi < size(); qi++) {
    if (!flags()[qi].immovable) {
      f_sum()[qi] += f()[qi];
      q()[qi] -= solve(H()[qi], f()[qi]) * 0.5f;
    }
  }
  return true;
}

bool vbd_3d_qd::newton_step(const basic_color_data &color) {
  for (unsigned cq = 0; cq < color.cq_num; cq++) {
    unsigned qi = color.to_q[cq];
    if (!flags()[qi].immovable) {
      f_sum()[qi] += f()[qi];
      q()[qi] -= solve(H()[qi], f()[qi]);
    }
  }
  return true;
}
} // namespace cp6

