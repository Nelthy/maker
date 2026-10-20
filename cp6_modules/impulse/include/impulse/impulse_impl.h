#pragma once
#include "avbd_basic/vbd_3d_qd.h"
#include "collision_basic/contact_set.h"
#include "impulse_states.h"
#include "maths.h"
#include <iostream>

#define LOG(n) std::cout << #n << ": " << n << std::endl;

namespace cp6 {

static float bounce = .5;
static float imp_friction_coef = 0.5;
static float impulse_thresh = 10;

inline vec3f get_Jn(unsigned q, const vbd_3d_qd &qd_data, vec2f world_point, vec2f normal) {
  return vec3f{normal.x, normal.y, cross(world_point - qd_data.q_old()[q].xy(), normal)};
};

inline vec3f get_Jt(unsigned q, const vbd_3d_qd &qd_data, vec2f world_point, vec2f tangent) {
  return vec3f{tangent.x, tangent.y, cross(world_point - qd_data.q_old()[q].xy(), tangent)};
};

inline bool use_impulse(
  unsigned int qa,
  unsigned int qb,
  unsigned int cnt,
  const cp6::vbd_3d_qd &qa_data,
  const cp6::vbd_3d_qd &qb_data,
  const contact_2p_set &cnt_data
) {
  return true;

  vec3f Jn_a = get_Jn(
    qa,
    qa_data,
    cnt_data[cnt].cnt1.x1,
    cnt_data[cnt].cnt1.n
  );
  vec3f Jn_b = get_Jn(
    qb,
    qb_data,
    cnt_data[cnt].cnt1.x2,
    cnt_data[cnt].cnt1.n
  );

  float rel_vel1 = dot(Jn_b, qb_data.q_dot()[qb]) - dot(Jn_a, qa_data.q_dot()[qa]);

  float rel_vel2 = 0;
  if (cnt_data[cnt].cnt2.is_valid()) {
    Jn_a = get_Jn(
      qa,
      qa_data,
      cnt_data[cnt].cnt2.x1,
      cnt_data[cnt].cnt2.n
    );

    Jn_b = get_Jn(
      qb,
      qb_data,
      cnt_data[cnt].cnt2.x2,
      cnt_data[cnt].cnt2.n
    );

    rel_vel2 = dot(Jn_b, qb_data.q_dot()[qb]) - dot(Jn_a, qa_data.q_dot()[qa]);

    return min(rel_vel1, rel_vel2) < -impulse_thresh;
  } else {
    return rel_vel1 < -impulse_thresh;
  }
};

inline vec3f get_inv_M(unsigned q, const vbd_3d_qd &q_data) {
  if (!q_data.flags()[q].immovable) {
    vec3f mass = q_data.mass()[q];
    return {1.f / mass.x, 1.f / mass.y, 1.f / mass.z};
  } else {
    return {0, 0, 0};
  }
}

inline vec3f get_v(unsigned q, const vbd_3d_qd &q_data) {
  // this is some cheese to make it work with avbd.
  return q_data.q_dot()[q] * bounce + q_data.impulse()[q];
}

inline vec3f get_v_start(unsigned q, const vbd_3d_qd &q_data) { return q_data.q_dot()[q]; }

inline void init_impulse_data(
  imp_data_2p &imp_data,
  unsigned qa,
  unsigned qb,
  unsigned imp,
  unsigned cnt,
  const vbd_3d_qd &qa_data,
  const vbd_3d_qd &qb_data,
  const contact_2p_set &cnt_data
) {

  imp_data.impulses()[imp].impulse_a.n = cnt_data[cnt].cnt1.n;
  imp_data.impulses()[imp].impulse_a.x1 = cnt_data[cnt].cnt1.x1;
  imp_data.impulses()[imp].impulse_a.x2 = cnt_data[cnt].cnt1.x2;

  imp_data.impulses()[imp].impulse_b.n = cnt_data[cnt].cnt2.n;
  imp_data.impulses()[imp].impulse_b.x1 = cnt_data[cnt].cnt2.x1;
  imp_data.impulses()[imp].impulse_b.x2 = cnt_data[cnt].cnt2.x2;
  auto Jn_a = get_Jn(
    qa,
    qa_data,
    imp_data.impulses()[imp].impulse_a.x1,
    imp_data.impulses()[imp].impulse_a.n
  );
  auto Jn_b = -get_Jn(
    qb,
    qb_data,
    imp_data.impulses()[imp].impulse_a.x2,
    imp_data.impulses()[imp].impulse_a.n
  );
  imp_data.impulses()[imp].impulse_a.rel_v_start =
    dot(Jn_b, get_v_start(qb, qb_data)) - dot(Jn_a, get_v_start(qa, qa_data));

  Jn_a = get_Jn(
    qa,
    qa_data,
    imp_data.impulses()[imp].impulse_b.x1,
    imp_data.impulses()[imp].impulse_b.n
  );
  Jn_b = -get_Jn(
    qb,
    qb_data,
    imp_data.impulses()[imp].impulse_b.x2,
    imp_data.impulses()[imp].impulse_b.n
  );
  imp_data.impulses()[imp].impulse_b.rel_v_start =
    dot(Jn_b, get_v_start(qb, qb_data)) - dot(Jn_a, get_v_start(qa, qa_data));
};

template<typename QA_DATA, typename QB_DATA>
inline void add_impulse_impl(
  QA_DATA &qa_data,
  QB_DATA &qb_data,
  unsigned int qa,
  unsigned int qb,
  impulse_contact &imp
) {
  if (qa_data.flags()[qa].dont_resolve_collisions || qb_data.flags()[qb].dont_resolve_collisions) {
    return;
  }
  // 1. get jacobian, mass, velocity
  // 2. calculate lambda
  // 3. clamp lambda and apply impulse

  // 1. get jacobian, mass, velocity
  const auto Jn_a = get_Jn(qa, qa_data, imp.x1, imp.n);
  const auto Jt_a = get_Jt(qa, qa_data, imp.x1, perp_l(imp.n));
  const auto Jn_b = -get_Jn(qb, qb_data, imp.x2, imp.n);
  const auto Jt_b = -get_Jt(qb, qb_data, imp.x2, perp_l(imp.n));
  const auto Jn = concat(Jn_a, Jn_b);
  const auto Jt = concat(Jt_a, Jt_b);

  const auto inv_M_a = get_inv_M(qa, qa_data);
  const auto inv_M_b = get_inv_M(qb, qb_data);
  const auto inv_M = concat(inv_M_a, inv_M_b);

  const auto v = concat(get_v(qa, qa_data), get_v(qb, qb_data));

  // 2. calculate lambda
  float lambda_n;
  {
    // normal
    auto WJn = Jn;
    scale_left(WJn, inv_M);
    // mc = 1 / J * W * J^T
    float mc = 1.f / dot(Jn, WJn);
    // lambda_n = mc * (dot(Jn, v) -   rel_vn);
    lambda_n = mc * (dot(Jn, v));
  }

  float lambda_t;
  {
    // tangent
    auto WJt = Jt;
    scale_left(WJt, inv_M);
    float mc = 1.f / dot(Jt, WJt);
    lambda_t = mc * (dot(Jt, v));
  }

  // 3. clamp lambda and apply impulse
  {
    // normal
    float total_impulse_start = imp.total_impulse_n;
    imp.total_impulse_n += lambda_n;
    if (imp.total_impulse_n < 0) {
      imp.total_impulse_n = 0;
    }
    float delta_impulse_n = imp.total_impulse_n - total_impulse_start;
    auto Pn_a = -Jn_a * delta_impulse_n;
    scale_left(Pn_a, inv_M_a);

    auto Pn_b = -Jn_b * delta_impulse_n;
    scale_left(Pn_b, inv_M_b);

    apply_impulse(qa, qa_data, Pn_a);
    apply_impulse(qb, qb_data, Pn_b);
  }

  {
    // tangent
    float total_impulse_start = imp.total_impulse_t;
    imp.total_impulse_t += lambda_t;
    imp.total_impulse_t = clamp(
      imp.total_impulse_t,
      -imp.total_impulse_n * imp_friction_coef,
      imp.total_impulse_n * imp_friction_coef
    );

    float delta_impulse_t = imp.total_impulse_t - total_impulse_start;
    auto Pt_a = -Jt_a * delta_impulse_t;
    scale_left(Pt_a, inv_M_a);

    auto Pt_b = -Jt_b * delta_impulse_t;
    scale_left(Pt_b, inv_M_b);

    apply_impulse(qa, qa_data, Pt_a);
    apply_impulse(qb, qb_data, Pt_b);
  }
  // The jacobian actually stays the same throughout the iteration, so recalculating it could be a
  // waste.
  //
  // The jacobian here is the ex1ct same as the one in avbd.
  // The lambdas between here and avbd are likely transferable through a function.
  // The contacts between here and avbd share a lot of state.
}

template<typename QA_DATA, typename QB_DATA>
inline void add_impulse(
  QA_DATA &qa_data,
  QB_DATA &qb_data,
  unsigned int qa,
  unsigned int qb,
  unsigned int imp,
  imp_data_2p &imp_data
) {
  add_impulse_impl(qa_data, qb_data, qa, qb, imp_data.impulses()[imp].impulse_a);

  if (imp_data.impulses()[imp].impulse_b.is_valid()) {
    add_impulse_impl(qa_data, qb_data, qa, qb, imp_data.impulses()[imp].impulse_b);
  }
}

inline void apply_impulse(unsigned int q, cp6::vbd_3d_qd &q_data, vec3f impulse) {
  q_data.impulse()[q] += impulse;
}
}; // namespace cp6
