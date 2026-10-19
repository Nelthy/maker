#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/avbd_system.h"
#include "avbd_basic/liquid/liquid_states.h"
#include "avbd_basic/liquid/vbd_liquid_qd.h"
#include "collision/narrowphase_system.h"
#include "god_state.h"
#include "cubic_spline_kernal_fns.h"
#include "maths.h"
#include "region_state.h"
#include "spiky_kernel_fns.h"
#include "tuple_utils.h"

namespace cp6 {
// liquid mass scaler
inline static float lms = 1e-1;
inline static float liquid_init_k = 5'000'000'000 * lms; // 500'000'001;
inline static float liquid_gamma = 0.0;
inline static float liquid_beta = 4'000'000 * lms;
inline static float liquid_alpha = .95;
inline static bool liquid_avbd = false;

static float liquid_kernel_rad = 60;
static float liquid_target_rho = 0.3 * lms;

inline void add_liquid_derivs(vbd_liquid_qd &qd, unsigned q) {
  liquid_vars &ld = qd.liquid_data()[q];

  float rho = 0;
  // pa_C_q = 1/p0 * sum{m_j * pa_Wij_q}
  vec2f pa_C_q = {0, 0};
  // pa_C_qq = 1/p0 * sum{m_j * pa_Wij_qq}
  mat2f pa_C_qq = {0, 0, 0, 0};
  ld.C = 0;

  // calculating the sums of pa_C_q and pa_C_qq and the density, rho
  for (unsigned q_oq = 0; q_oq < ld.adj.size(); q_oq++) {
    unsigned oq = ld.adj[q_oq];

    vec2f r_vec = qd.q()[q] - qd.q()[oq];
    float r = length(r_vec);
    if (r <= 1e-7) {
      continue;
    }
    r_vec /= r;

    const float W = spiky_w(r, liquid_kernel_rad);
    const vec2f W_grad = spiky_grad(r_vec, r, liquid_kernel_rad);
    const mat2f W_hess = spiky_hess(r_vec, r, liquid_kernel_rad);
    const float mj = qd.mass()[oq].x;

    rho += W * mj;
    pa_C_q += W_grad * mj;
    pa_C_qq += W_hess * mj;
  }

  pa_C_q *= 1.f / liquid_target_rho;
  pa_C_qq *= 1.f / liquid_target_rho;
  ld.C = ((rho / liquid_target_rho) - 1) * (1 - liquid_alpha);

  // clamp C to stop clumping
  // if (ld.C < 0) {
  //   ld.C = 0;
  // }

  float force = ld.C * ld.k;

  if (liquid_avbd) {
    force += ld.lambda;
  }

  qd.f()[q] += pa_C_q * force;

  vec2f G = vec2f{length(pa_C_qq.col(0)), length(pa_C_qq.col(1))} * abs(force);
  mat2f H;
  {
    H = outer(pa_C_q, pa_C_q);
    H *= ld.k;
    diag_sum(H, G);
  }

  qd.H()[q] += H;
}

template<typename TW>
struct avbd_liquid_fg_tt: cp6::flag_avbd_jacobi_fg{
  static constexpr unsigned lqt_num = get_matching_num<is_vbd_lqt, qt_ts<TW>>();
  static constexpr unsigned qt = generate_tuple_indirection<
    get_matching_ts<is_vbd_lqt, qt_ts<TW>>,
    qt_ts<TW>,
    std::is_same
  >()[0];
  static constexpr auto vbd_qt_to_qt = generate_tuple_indirection<
    get_matching_ts<is_vbd_qt, qt_ts<TW>>,
    qt_ts<TW>,
    std::is_same
  >();
  static_assert(lqt_num == 1);

private:
  friend class god::state<TW>;
  friend class avbd_system_tt<TW>;

  void start_step(unsigned reg, float h) {
    auto &reg_s = get<is_region_state>(*m_state);
    vbd_liquid_qd &q_data = reg_s.template get_qt<qt>(reg).qd();

    // update adjacency
    const auto &np_s = get<narrowphase_system>(*m_state);
    const auto &tcnt_data = np_s.template get_tcnt_data<qt, qt>(reg);

    for (unsigned q = 0; q < q_data.size(); q++) {
      q_data.liquid_data()[q].adj.clear();
    }
    for (unsigned tcnt = 0; tcnt < tcnt_data.size(); tcnt++) {
      unsigned qa = tcnt_data.get_q1(tcnt);
      unsigned qb = tcnt_data.get_q2(tcnt);
      q_data.liquid_data()[qa].adj.push_back(qb);
      q_data.liquid_data()[qb].adj.push_back(qa);
    }
    // unsigned ratio = edges.size() / q_data.size();

    if (liquid_avbd) {
      // init avbd vars
      for (unsigned q = 0; q < q_data.size(); q++) {
        liquid_vars &lv = q_data.liquid_data()[q];
        lv.k = max(liquid_init_k, lv.k * liquid_gamma);
        lv.lambda *= liquid_alpha * liquid_gamma;
      }
    } else {
      // set k
      for (unsigned q = 0; q < q_data.size(); q++) {
        liquid_vars &lv = q_data.liquid_data()[q];
        lv.k = liquid_init_k;
      }
    }
  };

  void add_derivs(unsigned reg) {
    auto &reg_s = get<is_region_state>(*m_state);
    auto &q_data = reg_s.template get_qt<qt>(reg);
    for (unsigned q = 0; q < q_data.size(); q++) {
      add_liquid_derivs(q_data, q);
    }
  }

  //
  // template<unsigned VBD_QT>
  // void add_derivs_gs(unsigned reg, unsigned q) {
  //   static constexpr unsigned qt_ = vbd_qt_to_qt[VBD_QT];
  //   if constexpr (qt_ == qt) {
  //     auto &reg_s = get<is_region_state>(*m_state);
  //     auto &q_data = reg_s.template get_qt<qt>(reg);
  //     add_liquid_derivs(q_data, q);
  //   }
  // }

  void after_step(unsigned reg, float h) {
    auto &reg_s = get<is_region_state>(*m_state);
    auto &q_data = reg_s.template get_qt<qt>(reg);

    if (liquid_avbd) {
      for (unsigned q = 0; q < q_data.size(); q++) {
        liquid_vars &lv = q_data.liquid_data()[q];
        lv.lambda += lv.C * lv.k;
        lv.k += liquid_beta * abs(lv.C);
      }
    }
  }

  void end_step(unsigned reg, float h) {}

  void init(god::state<TW> &state) { m_state = &state; }

  liquid_state l_state;
  god::state<TW> *m_state;
};

struct avbd_liquid_fg {

  template<typename TW>
  using template_t = avbd_liquid_fg_tt<TW>;
};
} // namespace cp6

/*
  THOUGHTS.
  I don't know a lot about liquid, so lets just get it working with a single qd before generalizing
  to to multiple.
*/

// inline void add_liquid_derivs(vbd_liquid_qd &qd, unsigned q) {
//   liquid_vars &ld = qd.liquid_data()[q];
//
//   // J is the density ratio between the old and new density
//   float J = 0;
//   vec2f pa_J_q = {0, 0};
//   mat2f pa_J_qq{0, 0, 0, 0};
//
//   if (ld.adj.size() == 0) {
//     return;
//   }
//
//   for (unsigned q_oq = 0; q_oq < ld.adj.size(); q_oq++) {
//     unsigned oq = ld.adj[q_oq];
//
//     vec2f r_vec = qd.q()[oq] - qd.q()[q];
//     float r = length(r_vec);
//     r_vec /= r;
//
//     float W = spiky_w(r, h_rad);
//     if (W < 1e-6) {
//       continue;
//     }
//     // LOG(W)
//     vec2f W_grad = spiky_grad(r_vec, r, h_rad);
//     mat2f W_hess = spiky_hess(r_vec, r, h_rad);
//     float mj = qd.mass()[oq].x;
//
//     J += 1.f / (mj * W);
//     pa_J_q += W_grad / (mj * W * W);
//     pa_J_qq += ((outer(W_grad, W_grad) * 2.f) / (mj * W * W * W)) - ((W_hess / (mj * W * W)));
//   }
//   float rho0 = 1;//qd.liquid_data()[q].intial_density;
//   float k = 1;
//   J *= rho0;
//   pa_J_q *= -rho0;
//   pa_J_qq *= -rho0;
//   LOG(J)
//   ld.C = (J - 1);
//   float force = ld.C * k; // + ld.lambda;
//   qd.f()[q] += pa_J_q * force;
//
//   // pa_C_qq = pa_J_qq
//   vec2f G = vec2f{length(pa_J_qq.col(0)), length(pa_J_qq.col(1))} * abs(force);
//   mat2f H;
//   {
//     H = outer(pa_J_q, pa_J_q);
//     H *= k;
//     diag_sum(H, G);
//   }
//   qd.H()[q] += H;
// };
