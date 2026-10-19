#pragma once
// Math copied and modifed from avbd 2d demo

/*
 * Copyright (c) 2025 Chris Giles
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies.
 * Chris Giles makes no representations about the suitability
 * of this software for any purpose.
 * It is provided "as is" without express or implied warranty.
 */

// should only be included in cpp files
#include "avbd_basic/constraint/avbd_pin_fg.h"
#include "avbd_basic/vbd_3d_qd.h"

namespace cp6 {
static float pin_k = 0;
static float pin_alpha = 0.5;
static float pin_beta = 1'000;
static float pin_gamma = .99;

template<typename QD_A, typename QD_B>
void init_pf_avbd_vars(
  dlm::vec_ds<cp6::pin_data> &pf_state,
  unsigned q_a,
  unsigned q_b,
  unsigned pf,
  const QD_A &qd_a,
  const QD_B &qd_b
) {
  cp6::pin_data &pin = pf_state[pf];
  vec2f world_a = transform_point(q_a, qd_a, pin.ra);
  vec2f world_b = transform_point(q_b, qd_b, pin.rb);

  pin.C0 = world_a - world_b;

  pin.k[0] =
    std::max(std::min(qd_a.mass()[q_a].x, qd_a.mass()[q_b].x) * pin_k, pin.k[0] * pin_gamma);
  pin.k[0] = pin.k[1];
  pin.lambda *= pin_alpha * pin_gamma;
}

template<bool A, typename OTHER_QD>
inline void add_pin_derivs_impl(
  vbd_3d_qd &dst_qd,
  const OTHER_QD &other_qd,
  unsigned int q,
  unsigned int other_q,
  const cp6::pin_data &pin
) {
  if (dst_qd.flags()[q].immovable) {
    return;
  }

  // compute C
  vec2f C;
  if constexpr (A) {
    C = transform_point(q, dst_qd, pin.ra) - transform_point(other_q, other_qd, pin.rb);
  } else {
    C = transform_point(other_q, other_qd, pin.ra) - transform_point(q, dst_qd, pin.rb);
  }

  C -= pin.C0 * pin_alpha;

  // compute force
  vec2f f;
  f[0] = C[0] * pin.k[0] + pin.lambda[0];
  f[1] = C[1] * pin.k[1] + pin.lambda[1];

  // compute derivs
  vec3f J[2];
  mat3f H[2];
  if constexpr (A) {
    vec2f r = rotate(pin.ra, dst_qd.q()[q].z);
    J[0] = {1.0f, 0.0f, -r.y};
    J[1] = {0.0f, 1.0f, r.x};
    H[0] = {0, 0, 0, 0, 0, 0, 0, 0, -r.x};
    H[1] = {0, 0, 0, 0, 0, 0, 0, 0, -r.y};

  } else {
    vec2f r = rotate(pin.rb, dst_qd.q()[q].z);
    J[0] = {-1.0f, 0.0f, r.y};
    J[1] = {0.0f, -1.0f, -r.x};
    H[0] = {0, 0, 0, 0, 0, 0, 0, 0, -r.x};
    H[1] = {0, 0, 0, 0, 0, 0, 0, 0, -r.y};
  }

  // add derivs
  vec3f G[2];
  G[0] = vec3f{length(H[0].col(0)), length(H[0].col(1)), length(H[0].col(2))} * abs(f[0]);
  G[1] = vec3f{length(H[1].col(0)), length(H[1].col(1)), length(H[1].col(2))} * abs(f[1]);

  dst_qd.f()[q] += J[0] * f[0];
  dst_qd.f()[q] += J[1] * f[1];

  mat3f H0;
  {
    H0 = outer(J[0], J[0]);
    H0 *= pin.k[0];
    diag_sum(H0, G[0]);
  }

  dst_qd.H()[q] += H0;

  mat3f H1;
  {
    H1 = outer(J[1], J[1]);
    H1 *= pin.k[1];
    diag_sum(H1, G[1]);
  }
  dst_qd.H()[q] += H1;
}

// template<bool A, typename OTHER_QD>
// inline void add_pf_derivs_impl_spring(
//   vbd_3d_qd &dst_qd,
//   const OTHER_QD &other_qd,
//   unsigned int q,
//   unsigned int other_q,
//   const cp6::pin_data &pin
// ) {
//   if (dst_qd.flags()[q].immovable) {
//     return;
//   }
//   // Compute the first and second derivatives for the desired body
//   mat2f S = {0, -1, 1, 0};
//   mat2f I = {1, 0, 0, 1};
//
//   // vec2f delta = transform(bodyA->position, rA) - transform(bodyB->position, rB);
//   vec2f d;
//   if constexpr (A) {
//     d = transform_point(q, dst_qd, pin.ra) - transform_point(other_q, other_qd, pin.rb);
//   } else {
//     d = transform_point(other_q, other_qd, pin.ra) - transform_point(q, dst_qd, pin.rb);
//   }
//
//   float dlen2 = dot(d, d);
//   if (dlen2 == 0) return;
//
//   float dlen = sqrtf(dlen2);
//   vec2f n = d / dlen;
//   mat2f dxx = (I - outer(n, n)) / dlen;
//
//   float C = dlen; // - pin.C0 * pin_alpha;
//
//   if constexpr (A) {
//     vec2f Sr = rotate(S * pin.ra, dst_qd.q()[q].z);
//     vec2f r = rotate(pin.ra, dst_qd.q()[q].z);
//
//     vec2f dxr = dxx * Sr;
//     float drr = -dot(n, r) - dot(n, r);
//     vec3f J;
//     J.xy() = n;
//     J.z = dot(n, Sr);
//     mat3f H =
//       {dxx.row[0].x, dxx.row[0].y, dxr.x, dxx.row[1].x, dxx.row[1].y, dxr.y, dxr.x, dxr.y, drr};
//
//     float f = C * pin.k + pin.lambda;
//     mat3f G = diagonal(length(H.col(0)), length(H.col(1)), length(H.col(2))) * abs(f);
//
//     dst_qd.f()[q] += J * f;
//     dst_qd.H()[q] += outer(J, J) * pin.k + G;
//   } else {
//     vec2f Sr = rotate(S * pin.rb, dst_qd.q()[q].z);
//     vec2f r = rotate(pin.rb, dst_qd.q()[q].z);
//     vec2f dxr = dxx * Sr;
//     float drr = dot(n, r) + dot(n, r);
//
//     vec3f J;
//     J.xy() = -n;
//     J.z = dot(n, -Sr);
//     mat3f H =
//       {dxx.row[0].x, dxx.row[0].y, dxr.x, dxx.row[1].x, dxx.row[1].y, dxr.y, dxr.x, dxr.y, drr};
//
//     float f = C * pin.k + pin.lambda;
//     mat3f G = diagonal(length(H.col(0)), length(H.col(1)), length(H.col(2))) * abs(f);
//
//     dst_qd.f()[q] += J * f;
//     dst_qd.H()[q] += outer(J, J) * pin.k + G;
//   }
// }

template<typename OTHER_QD>
inline void add_pf_derivs_a(
  vbd_3d_qd &dst_qd,
  const OTHER_QD &other_qd,
  unsigned int q,
  unsigned int other_q,
  unsigned int pf,
  const dlm::vec_ds<cp6::pin_data> &pf_state
) {
  add_pin_derivs_impl<true>(dst_qd, other_qd, q, other_q, pf_state[pf]);
};

template<typename OTHER_QD>
inline void add_pf_derivs_b(
  vbd_3d_qd &dst_qd,
  const OTHER_QD &other_qd,
  unsigned int q,
  unsigned int other_q,
  unsigned int pf,
  const dlm::vec_ds<cp6::pin_data> &pf_state
) {
  add_pin_derivs_impl<false>(dst_qd, other_qd, q, other_q, pf_state[pf]);
};

template<typename QD_A, typename QD_B>
void update_pf_avbd_vars(
  dlm::vec_ds<cp6::pin_data> &pf_state,
  unsigned q_a,
  unsigned q_b,
  unsigned pf,
  const QD_A &qd_a,
  const QD_B &qd_b
) {
  cp6::pin_data &pin = pf_state[pf];
  vec2f C = transform_point(q_a, qd_a, pin.ra) - transform_point(q_b, qd_b, pin.rb);
  pin.lambda[0] += pin.k[0] * C[0];
  pin.lambda[1] += pin.k[1] * C[1];
  pin.k[0] += pin_beta * abs(C[0]);
  pin.k[1] += pin_beta * abs(C[1]);
}
} // namespace cp6
