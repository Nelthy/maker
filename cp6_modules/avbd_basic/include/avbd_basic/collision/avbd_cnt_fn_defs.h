#pragma once
#include "avbd/collision/avbd_pcnt_fg.h"
#include "avbd_basic/collision/basic_q_to_cnt.h"
#include "avbd_basic/vbd_2d_qd.h"
#include "avbd_basic/vbd_3d_qd.h"
#include "avbd_contact_set.h"
#include "collision_basic/contact_set.h"
#include "maths.h"
// should only be included in cpp files.

namespace cp6 {
static float cnt_friction_coef = .5;
static float cnt_col_thresh = 1;
static float cnt_stick_thresh = 1;

inline vec2f get_relative_point(const vbd_3d_qd &qd, unsigned q, vec2f avbd_cnt_point) {
  return rotate(avbd_cnt_point, qd.q_old()[q].z);
}

inline vec2f get_relative_point(const vbd_2d_qd &qd, unsigned q, vec2f avbd_cnt_point) {
  return avbd_cnt_point;
}

inline vec3f get_contact_Jn(unsigned q, vbd_3d_qd &qd, vec2f relative_point, vec2f n) {
  return vec3f{n.x, n.y, cross(relative_point, n)};
}

inline vec3f get_contact_Jt(unsigned q, vbd_3d_qd &qd, vec2f relative_point, vec2f n) {
  vec2f t = perp_l(n);
  return vec3f{t.x, t.y, cross(relative_point, t)};
}

inline vec2f get_contact_Jn(unsigned q, vbd_2d_qd &qd, vec2f relative_point, vec2f n) {
  return vec2f{n.x, n.y};
}

inline vec2f get_contact_Jt(unsigned q, vbd_2d_qd &qd, vec2f relative_point, vec2f n) {
  return perp_l(n);
}

inline vec2f get_C_talor_contribution1(const vbd_3d_qd &qd, const avbd_contact &c, unsigned q) {
  const vec2f relative_point = get_relative_point(qd, q, c.x1);
  const vec3f pa_C0n_q_old = vec3f{c.n.x, c.n.y, cross(relative_point, c.n)};
  const vec2f tangent = perp_l(c.n);
  const vec3f pa_C0t_q_old = vec3f{tangent.x, tangent.y, cross(relative_point, tangent)};
  const vec3f delta_q = qd.q()[q] - qd.q_old()[q];
  return vec2f{dot(pa_C0n_q_old, delta_q), dot(pa_C0t_q_old, delta_q)};
}

inline vec2f get_C_talor_contribution2(const vbd_3d_qd &qd, const avbd_contact &c, unsigned q) {
  const vec2f relative_point = get_relative_point(qd, q, c.x2);
  const vec3f pa_C0n_q_old = -vec3f{c.n.x, c.n.y, cross(relative_point, c.n)};
  const vec2f tangent = perp_l(c.n);
  const vec3f pa_C0t_q_old = -vec3f{tangent.x, tangent.y, cross(relative_point, tangent)};
  const vec3f delta_q = qd.q()[q] - qd.q_old()[q];
  return vec2f{dot(pa_C0n_q_old, delta_q), dot(pa_C0t_q_old, delta_q)};
}

inline vec2f get_C_talor_contribution1(const vbd_2d_qd &qd, const avbd_contact &c, unsigned q) {
  const vec2f pa_C0n_q_old = vec2f{c.n.x, c.n.y};
  const vec2f tangent = perp_l(c.n);
  const vec2f pa_C0t_q_old = vec2f{tangent.x, tangent.y};
  const vec2f delta_q = qd.q()[q] - qd.q_old()[q];
  return vec2f{dot(pa_C0n_q_old, delta_q), dot(pa_C0t_q_old, delta_q)};
}

inline vec2f get_C_talor_contribution2(const vbd_2d_qd &qd, const avbd_contact &c, unsigned q) {
  const vec2f pa_C0n_q_old = -vec2f{c.n.x, c.n.y};
  const vec2f tangent = perp_l(c.n);
  const vec2f pa_C0t_q_old = -vec2f{tangent.x, tangent.y};
  const vec2f delta_q = qd.q()[q] - qd.q_old()[q];
  return vec2f{dot(pa_C0n_q_old, delta_q), dot(pa_C0t_q_old, delta_q)};
}

inline float get_mass(unsigned q, const vbd_3d_qd &qd) { return qd.mass()[q].x; }

inline float get_mass(unsigned q, const vbd_2d_qd &qd) { return qd.mass()[q].x; }

// implementation functions

template<typename QD1, typename QD2>
inline void init_avbd_state_impl(
  const QD1 &qd1,
  const QD2 &qd2,
  unsigned q1,
  unsigned q2,
  const contact &src_contact,
  avbd_contact &dst_contact,
  const avbd_constants &constants
) {
  const float gamma = constants.gamma;
  const float alpha = constants.alpha;
  const float k = constants.k;
  if (!src_contact.is_valid()) {
    dst_contact.invalidate();
    assert(!dst_contact.is_valid());
    return;
  }

  dst_contact.k[0] =
    std::max(std::min(get_mass(q1, qd1), get_mass(q2, qd2)) * k, dst_contact.k[0] * gamma);

  dst_contact.k[1] =
    std::max(std::min(get_mass(q1, qd1), get_mass(q2, qd2)) * k, dst_contact.k[1] * gamma);

  vec2f body1 = inv_transform_point_old(q1, qd1, src_contact.x1);
  vec2f body2 = inv_transform_point_old(q2, qd2, src_contact.x2);
  static constexpr float point_thresh = 1;
  if (dst_contact.stick) {
    dst_contact.n = src_contact.n;
  } else {
    dst_contact.n = src_contact.n;
    dst_contact.x1 = body1;
    dst_contact.x2 = body2;
  }

  vec2f world1 = transform_point_old(q1, qd1, dst_contact.x1); // src_contact.x1;
  vec2f world2 = transform_point_old(q2, qd2, dst_contact.x2); // src_contact.x2;

  // this is equivalent to the one in the demo
  vec2f C01 = {
    dot((world1 - world2), dst_contact.n) - cnt_col_thresh,
    dot((world1 - world2), perp_l(dst_contact.n))
  };
  // vec2f C02;
  // {
  //   vec2f n = dst_contact.n;
  //   vec2f t = perp_l(dst_contact.n);
  //   mat2f basis = {n.x, n.y, t.x, t.y};
  //
  //   vec2f r1 = rotate(dst_contact.x1, qd1.q()[q1].z);
  //   vec2f r2 = rotate(dst_contact.x2, qd1.q()[q2].z);
  //   vec2f p1 = qd1.q_old()[q1].xy();
  //   vec2f p2 = qd2.q_old()[q2].xy();
  //   C02 = basis * (p1 + r1 - p2 - r2) + vec2f{cnt_col_thresh, 0};
  // }
  // std::cout << "diff: " << C01 - C02 << std::endl;
  dst_contact.C0 = C01;
  // The basic implmementation currently has no way to mark cnt1 as changing places, so we just
  // have a threshold
  dst_contact.stick =
    (abs(dst_contact.lambda[1]) <= abs(dst_contact.lambda[0] * cp6::cnt_friction_coef)) &&
    (abs(dst_contact.C0[1]) < cnt_stick_thresh) &&
    length_squared(body1 - dst_contact.x1) < point_thresh * point_thresh &&
    length_squared(body2 - dst_contact.x2) < point_thresh * point_thresh;

  // std::cout << "lambda[0]: " << dst_contact.lambda[0] << std::endl;
  // std::cout << "lambda[1]: " << dst_contact.lambda[1] << std::endl;
  // std::cout << "C0[1]: " << dst_contact.C0[1] << std::endl;
  // std::cout << "stick: "<<std::boolalpha << dst_contact.stick << std::endl;
  // updating lambda
  dst_contact.lambda *= alpha * gamma;
}

template<bool A, typename DST_QD, typename OTHER_QD>
static inline void add_derivs_impl(
  DST_QD &dst_qd,
  const OTHER_QD &other_qd,
  unsigned q,
  unsigned other_q,
  const avbd_contact &c,
  const avbd_constants &constants
) {
  if (dst_qd.flags()[q].immovable || dst_qd.flags()[q].dont_resolve_collisions ||
      other_qd.flags()[other_q].dont_resolve_collisions) {
    return;
  }
  float alpha = constants.alpha;
  assert(c.is_valid());

  vec2f C;
  if constexpr (A) {
    C = c.C0 * (1 - alpha) + get_C_talor_contribution1(dst_qd, c, q) +
        get_C_talor_contribution2(other_qd, c, other_q);
  } else {
    C = c.C0 * (1 - alpha) + get_C_talor_contribution2(dst_qd, c, q) +
        get_C_talor_contribution1(other_qd, c, other_q);
  }

  vec2f relative_point;
  if constexpr (A) {
    relative_point = get_relative_point(dst_qd, q, c.x1);
  } else {
    relative_point = get_relative_point(dst_qd, q, c.x2);
  }

  // adding normal force
  {
    float force = C[0] * c.k[0] + c.lambda[0];

    if (force < 0) {
      force = 0;
    }

    auto J = get_contact_Jn(q, dst_qd, relative_point, c.n);
    if constexpr (!A) {
      J *= -1;
    }

    dst_qd.f()[q] += J * force;
    dst_qd.H()[q] += outer(J, J) * c.k[0];
  }

  // adding tangent force
  {
    float force = C[1] * c.k[1] + c.lambda[1];
    float friction_bound = abs(c.lambda[0]) * cnt_friction_coef;
    force = clamp(force, -friction_bound, friction_bound);

    const vec2f tangent = perp_l(c.n);
    auto J = get_contact_Jt(q, dst_qd, relative_point, c.n);
    if constexpr (!A) {
      J *= -1;
    }
    dst_qd.f()[q] += J * force;
    dst_qd.H()[q] += outer(J, J) * c.k[1];
  }
}

template<typename QD1, typename QD2>
void update_avbd_vars_impl(
  avbd_contact &c,
  unsigned q1,
  unsigned q2,
  const QD1 &qd1,
  const QD2 &qd2,
  const avbd_constants &constants
) {
  const float alpha = constants.alpha;
  const float beta = constants.beta;
  vec2f C = c.C0 * (1 - alpha) + get_C_talor_contribution1(qd1, c, q1) +
            get_C_talor_contribution2(qd2, c, q2);
  // clamping normal lambda
  c.lambda[0] += c.k[0] * C[0];
  c.lambda[1] += c.k[1] * C[1];
  if (c.lambda[0] < 0) {
    c.lambda[0] = 0;
  } else {
    c.k[0] += beta * std::abs(C[0]);
  }

  // clamping tangent lambda
  float friction_bound = abs(c.lambda[0]) * cnt_friction_coef;
  c.lambda[1] = clamp(c.lambda[1], -friction_bound, friction_bound);

  if (-friction_bound < c.lambda[1] && c.lambda[1] < friction_bound) {
    c.k[1] += beta * std::abs(C[1]);
  }
};

// avbd_contact_set_1 interface functions

template<typename QD1, typename QD2>
inline void init_avbd_state(
  const QD1 &qd1,
  const QD2 &qd2,
  const contact_1p_set &src_cnts,
  avbd_contact_1p_set &dst_cnts
) {
  assert(src_cnts.size() == dst_cnts.size());
  for (unsigned cnt = 0; cnt < src_cnts.size(); cnt++) {
    dst_cnts[cnt].q1 = src_cnts[cnt].q1;
    dst_cnts[cnt].q2 = src_cnts[cnt].q2;
    init_avbd_state_impl(
      qd1,
      qd2,
      dst_cnts[cnt].q1,
      dst_cnts[cnt].q2,
      src_cnts[cnt],
      dst_cnts[cnt],
      dst_cnts
    );
  }
}

template<typename QD1, typename QD2>
inline void add_derivs1(QD1 &qd1, const QD2 &qd2, const avbd_contact_1p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    add_derivs_impl<true>(qd1, qd2, cnts[cnt].q1, cnts[cnt].q2, cnts[cnt], cnts.alpha);
  }
}

template<typename QD1, typename QD2>
inline void add_derivs2(const QD1 &qd1, QD2 &qd2, const avbd_contact_1p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    add_derivs_impl<false>(qd2, qd1, cnts[cnt].q2, cnts[cnt].q1, cnts[cnt], cnts.alpha);
  }
}

template<typename QD1, typename QD2>
inline void add_derivs1(
  QD1 &qd1,
  const QD2 &qd2,
  const avbd_contact_1p_set &cnts,
  const basic_color_data &color,
  const basic_q_to_cnt &q_to_cnt
) {}

template<typename QD1, typename QD2>
inline void add_derivs2(
  const QD1 &qd2,
  QD2 &qd1,
  const avbd_contact_1p_set &cnts,
  const basic_color_data &color,
  const basic_q_to_cnt &q_to_cnt
) {}

template<typename QD1, typename QD2>
inline void update_avbd_vars(const QD1 &qd1, const QD2 &qd2, avbd_contact_1p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    update_avbd_vars_impl(cnts[cnt], cnts[cnt].q1, cnts[cnt].q2, qd1, qd2, cnts);
  }
};

// avbd_contact_set_2p interface functions

template<typename QD1, typename QD2>
inline bool init_avbd_state(
  const QD1 &qd1,
  const QD2 &qd2,
  const contact_2p_set &src_cnts,
  avbd_contact_2p_set &dst_cnts
) {
  assert(src_cnts.size() == dst_cnts.size());
  for (unsigned cnt = 0; cnt < src_cnts.size(); cnt++) {
    dst_cnts[cnt].q1 = src_cnts[cnt].q1;
    dst_cnts[cnt].q2 = src_cnts[cnt].q2;
    init_avbd_state_impl(
      qd1,
      qd2,
      dst_cnts[cnt].q1,
      dst_cnts[cnt].q2,
      src_cnts[cnt].cnt1,
      dst_cnts[cnt].cnt1,
      dst_cnts
    );
    init_avbd_state_impl(
      qd1,
      qd2,
      dst_cnts[cnt].q1,
      dst_cnts[cnt].q2,
      src_cnts[cnt].cnt2,
      dst_cnts[cnt].cnt2,
      dst_cnts
    );
  }
  return true;
}

template<typename QD1, typename QD2>
inline bool add_derivs1(QD1 &qd1, const QD2 &qd2, const avbd_contact_2p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    add_derivs_impl<true>(qd1, qd2, cnts[cnt].q1, cnts[cnt].q2, cnts[cnt].cnt1, cnts);
    if (cnts[cnt].cnt2.is_valid()) {
      add_derivs_impl<true>(qd1, qd2, cnts[cnt].q1, cnts[cnt].q2, cnts[cnt].cnt2, cnts);
    }
  }
  return true;
}

template<typename QD1, typename QD2>
inline bool add_derivs2(const QD1 &qd1, QD2 &qd2, const avbd_contact_2p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    add_derivs_impl<false>(qd2, qd1, cnts[cnt].q2, cnts[cnt].q1, cnts[cnt].cnt1, cnts);
    if (cnts[cnt].cnt2.is_valid()) {
      add_derivs_impl<false>(qd2, qd1, cnts[cnt].q2, cnts[cnt].q1, cnts[cnt].cnt2, cnts);
    }
  }
  return true;
}

template<typename QD1, typename QD2>
inline bool add_derivs1(
  QD1 &qd1,
  const QD2 &qd2,
  const avbd_contact_2p_set &cnts,
  const basic_color_data &color,
  const basic_q_to_cnt &q_to_cnt
) {
  for (unsigned cq = 0; cq < color.cq_num; cq++) {
    unsigned q = color.to_q[cq];
    const unsigned *to_cnt = q_to_cnt.get_q_to_cnt(q);

    for (unsigned q_cnt = 0; q_cnt < q_to_cnt.q_cnt_num(q); q_cnt++) {
      unsigned cnt = to_cnt[q_cnt];
      assert(cnts[cnt].q1 == q);
      add_derivs_impl<true>(qd1, qd2, cnts[cnt].q1, cnts[cnt].q2, cnts[cnt].cnt1, cnts);
      if (cnts[cnt].cnt2.is_valid()) {
        add_derivs_impl<true>(qd1, qd2, cnts[cnt].q1, cnts[cnt].q2, cnts[cnt].cnt2, cnts);
      }
    }
  }
  return true;
}

template<typename QD1, typename QD2>
inline bool add_derivs2(
  const QD1 &qd1,
  QD2 &qd2,
  const avbd_contact_2p_set &cnts,
  const basic_color_data &color,
  const basic_q_to_cnt &q_to_cnt
) {
  for (unsigned cq = 0; cq < color.cq_num; cq++) {
    unsigned q = color.to_q[cq];
    const unsigned *to_cnt = q_to_cnt.get_q_to_cnt(q);
    for (unsigned q_cnt = 0; q_cnt < q_to_cnt.q_cnt_num(q); q_cnt++) {
      unsigned cnt = to_cnt[q_cnt];
      assert(cnts[cnt].q2 == q);
      add_derivs_impl<false>(qd2, qd1, cnts[cnt].q2, cnts[cnt].q1, cnts[cnt].cnt1, cnts);
      if (cnts[cnt].cnt2.is_valid()) {
        add_derivs_impl<false>(qd2, qd1, cnts[cnt].q2, cnts[cnt].q1, cnts[cnt].cnt2, cnts);
      }
    }
  }
  return true;
}

template<typename QD1, typename QD2>
inline bool update_avbd_vars(const QD1 &qd1, const QD2 &qd2, avbd_contact_2p_set &cnts) {
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    update_avbd_vars_impl(cnts[cnt].cnt1, cnts[cnt].q1, cnts[cnt].q2, qd1, qd2, cnts);
    update_avbd_vars_impl(cnts[cnt].cnt2, cnts[cnt].q1, cnts[cnt].q2, qd1, qd2, cnts);
  }
  return true;
};

inline bool update_q1_to_cnt(
  const avbd_contact_2p_set &cnts,
  basic_q_to_cnt &q_to_cnt,
  unsigned q1_num
) {
  q_to_cnt.init_storage(q1_num, cnts.size());
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    q_to_cnt.register_cnt(cnts[cnt].q1, cnt);
  }
  q_to_cnt.create_q_to_cnt();
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    q_to_cnt.proj_cnt(cnts[cnt].q1, cnt);
  }
  return true;
};

inline bool update_q2_to_cnt(
  const avbd_contact_2p_set &cnts,
  basic_q_to_cnt &q_to_cnt,
  unsigned q2_num
) {
  q_to_cnt.init_storage(q2_num, cnts.size());
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    q_to_cnt.register_cnt(cnts[cnt].q2, cnt);
  }
  q_to_cnt.create_q_to_cnt();
  for (unsigned cnt = 0; cnt < cnts.size(); cnt++) {
    q_to_cnt.proj_cnt(cnts[cnt].q2, cnt);
  }
  return true;
};
} // namespace cp6
