#pragma once
#include "maths.h"
#include <iostream>
static constexpr float cubic_hessian_eps = 1e-5f; // match your spiky eps

// normalization factor for cubic spline in 2D:
// W(r,h) = fac * w(q),  q = r/h,  fac = 10/(7*pi*h^2)
inline float cubic_c(float h) noexcept { return 10.0f / (7.0f * pi_v * h * h); }

// radial (dimensionless) basis w(q)
inline float cubic_w_q(float q) noexcept {
  if (q >= 2.0f) return 0.0f;
  if (q >= 1.0f) {
    const float tmp = 2.0f - q;
    return 0.25f * tmp * tmp * tmp; // 1/4 * (2-q)^3
  } else {
    // 1 - 1.5 q^2 + 0.75 q^3  (equivalently 1 - 1.5*q*q*(1 - 0.5*q))
    return 1.0f - 1.5f * q * q + 0.75f * q * q * q;
  }
}

// W(r,h)
inline float cubic_w(float r, float h) noexcept {
  if (r >= 2.0f * h) return 0.0f;
  const float q = r / h;
  const float fac = cubic_c(h);
  return fac * cubic_w_q(q);
}

inline float cubic_w(const vec2f &r_hat, float h) noexcept {
  const float r = std::sqrt(dot(r_hat, r_hat));
  return cubic_w(r, h);
}

// first derivative w.r.t q: dw/dq
inline float cubic_dw_dq(float q) noexcept {
  if (q >= 2.0f) return 0.0f;
  if (q >= 1.0f) {
    const float tmp = 2.0f - q;
    return -0.75f * tmp * tmp; // -3/4 * (2-q)^2
  } else {
    // derivative of 1 - 1.5 q^2 + 0.75 q^3  -> -3 q + 2.25 q^2
    // factorized as -3*q*(1 - 0.75*q)
    return -3.0f * q * (1.0f - 0.75f * q);
  }
}

// second derivative w.r.t q: d2w/dq2
inline float cubic_d2w_dq2(float q) noexcept {
  if (q >= 2.0f) return 0.0f;
  if (q >= 1.0f) {
    // derivative of -0.75*(2-q)^2 -> 1.5*(2-q)
    return 1.5f * (2.0f - q);
  } else {
    // derivative of -3 q + 2.25 q^2 -> -3 + 4.5 q
    return -3.0f + 4.5f * q;
  }
}

// radial derivatives W'(r) and W''(r)
// W'(r) = fac * (dw/dq) * (1/h)
// W''(r) = fac * (d2w/dq2) * (1/h^2)
inline float cubic_w_dr(float r, float h) noexcept {
  if (r >= 2.0f * h) return 0.0f;
  const float q = r / h;
  const float fac = cubic_c(h);
  return fac * cubic_dw_dq(q) / h;
}

inline float cubic_w_ddr(float r, float h) noexcept {
  if (r >= 2.0f * h) return 0.0f;
  const float q = r / h;
  const float fac = cubic_c(h);
  return fac * cubic_d2w_dq2(q) / (h * h);
}

// vector gradient: grad W = W'(r) * r_hat = (dW/dr) * r_hat / r
inline vec2f cubic_grad(const vec2f &r_hat, float r, float h) noexcept {
  if (r >= 2.0f * h) return vec2f{0.0f, 0.0f};
  if (r <= 0.0f) return vec2f{0.0f, 0.0f};
  const float dW_dr = cubic_w_dr(r, h);
  const float inv_r = 1.0f / r;
  return r_hat * (dW_dr * inv_r);
}

// 2D Hessian: Hess(W) = (W'' - W'/r) * r_hat⊗r_hat + (W'/r) * I
inline mat2f cubic_hess(const vec2f &r_hat, float r, float h) noexcept {
  // outside support -> zero matrix
  if (r >= 2.0f * h) {
    return mat2f{0.0f, 0.0f, 0.0f, 0.0f};
  }
  //
  // small-radius fallback: isotropic limit (W''(0)/2)*I in 2D
  if (r <= cubic_hessian_eps) {
    // compute W''(0):
    // q = 0 -> d2w/dq2 = -3  => W''(0) = fac * (-3) / h^2
    const float fac = cubic_c(h);
    const float w_ddr_0 = fac * (-3.0f) / (h * h);
    const float iso = 0.5f * w_ddr_0; // divide by d (2)
    return mat2f{iso, 0.0f, 0.0f, iso};
  }
  //
  // // general 0 < r < 2h
  // const float w_p = cubic_w_prime(r, h); // W'(r)
  // const float w_dd = cubic_w_ddr(r, h);  // W''(r)
  // const float inv_r = 1.0f / r;
  //
  // // r_hat components
  // const float rx = r_hat.x * inv_r;
  // const float ry = r_hat.y * inv_r;
  //
  // // outer product r_hat ⊗ r_hat
  // const float o_xx = rx * rx;
  // const float o_xy = rx * ry;
  // const float o_yy = ry * ry;
  //
  // // Hessian assembly
  // const float a = (w_dd - w_p * inv_r); // multiplier for outer product
  // const float b = w_p * inv_r;          // multiplier for identity
  //
  // const float Hxx = a * o_xx + b;
  // const float Hxy = a * o_xy;
  // const float Hyy = a * o_yy + b;
  //
  const float W_dr = cubic_w_dr(r, h);
  const float W_ddr = cubic_w_ddr(r, h);
  mat2f H = outer(r_hat, r_hat) * (W_ddr - (W_dr / r));
  H += identity<2>() * (W_dr / r);

  return H;
}

inline mat2f cubic_hess(const vec2f &r_hat, float h) noexcept {
  const float r = std::sqrt(dot(r_hat, r_hat));
  return cubic_hess(r_hat, r, h);
}
