#pragma once
#include "maths.h"

inline float spiky_c(float h) noexcept {
  // C2 = 10 / (pi * h^5)
  const float h2 = h * h;
  const float h4 = h2 * h2;
  const float h5 = h4 * h;
  return 10.0f / (pi_v * h5);
}

inline float spiky_w(float r, float h) noexcept {
  if (r >= h) return 0.0f;
  const float c = spiky_c(h);
  const float hr = h - r;
  return c * hr * hr * hr; // (h-r)^3
}

inline float spiky_w_dr(float r, float h) noexcept {
  if (r >= h) return 0.0f;
  const float c = spiky_c(h);
  const float hr = h - r;
  return -3.0f * c * hr * hr; // dW/dr
}

inline float spiky_w_ddr(float r, float h) noexcept {
  if (r >= h) return 0.0f;
  const float c = spiky_c(h);
  return 6.0f * c * (h - r); // d^2W/dr^2
}

inline vec2f spiky_grad(const vec2f &r_hat, float r, float h) noexcept {
  if (r >= h) return vec2f{0.0f, 0.0f};
  const float W_dr = spiky_w_dr(r, h);
  return r_hat * (W_dr / r);
}

inline mat2f spiky_hess(const vec2f &r_hat, float r, float h) noexcept {
  // outside support -> zero matrix
  if (r >= h) {
    return mat2f{0.0f, 0.0f, 0.0f, 0.0f};
  }
  
  // small-radius fallback: use isotropic limit H = (W''(0)/2) * I in 2D
  if (r <= 0e-5) {
    const float w_ddr_0 = spiky_w_ddr(r, h);
    const float iso = 0.5f * w_ddr_0; // divide by d (2)
    return mat2f{iso, 0.0f, 0.0f, iso};
  }

  // // general point 0 < r < h
  // const float w_prime = spiky_w_dr(r, h); // W'(r)
  // const float w_ddr = spiky_w_ddr(r, h);  // W''(r)
  // const float inv_r = 1.0f / r;
  //
  // // compute r_hat components
  // const float rx = r_vec.x * inv_r;
  // const float ry = r_vec.y * inv_r;
  //
  // // outer product r_hat ⊗ r_hat
  // const float o_xx = rx * rx;
  // const float o_xy = rx * ry;
  // const float o_yy = ry * ry;
  //
  // // build Hessian via: (W'' - W'/r)*(r_hat⊗r_hat) + (W'/r)*I
  // const float a = (w_ddr - w_prime * inv_r); // multiplier for outer
  // const float b = w_prime * inv_r;           // multiplier for identity
  //
  // const float Hxx = a * o_xx + b;
  // const float Hxy = a * o_xy;
  // const float Hyy = a * o_yy + b;

  // mat2f{Hxx, Hxy, Hxy, Hyy};

  const float W_dr = spiky_w_dr(r, h);
  const float W_ddr = spiky_w_ddr(r, h);
  mat2f H = outer(r_hat, r_hat) * (W_ddr - (W_dr / r));
  H += identity<2>() * (W_dr / r);

  return H;
}
