#pragma once
#include <cassert>
#include <cmath>

static constexpr float pi_v = 3.14159265358979323846f;

template<unsigned char D>
struct vecf {
  float dim[D];

  float &operator[](unsigned char i) {
    assert(i < D);
    return ((float *)this)[i];
  }

  const float &operator[](unsigned char i) const {
    assert(i < D);
    return ((float *)this)[i];
  }
};

template<>
struct vecf<2> {
  float x, y;

  float &operator[](unsigned char i) {
    assert(i < 2);
    return ((float *)this)[i];
  }

  const float &operator[](unsigned char i) const {
    assert(i < 2);
    return ((float *)this)[i];
  }
};

template<>
struct vecf<3> {
  float x, y, z;

  vecf<2> &xy() { return *(vecf<2> *)this; }

  const vecf<2> &xy() const { return *(vecf<2> *)this; }

  float &operator[](unsigned char i) {
    assert(i < 3);
    return ((float *)this)[i];
  }

  const float &operator[](unsigned char i) const {
    assert(i < 3);
    return ((float *)this)[i];
  }
};

using vec2f = vecf<2>;
using vec3f = vecf<3>;

template<unsigned char R, unsigned char C>
struct matf {
  vecf<C> row[R];

  vecf<C> &operator[](unsigned char i) {
    assert(i < R);
    return row[i];
  }

  const vecf<C> &operator[](unsigned char i) const {
    assert(i < R);
    return row[i];
  }

  vecf<R> col(unsigned c) const {
    vecf<R> out;
    for (unsigned i = 0; i < R; i++) {
      out[i] = row[i][c];
    }
    return out;
  }
};

using mat2f = matf<2, 2>;
using mat3f = matf<3, 3>;

// vecf operators

template<unsigned char D>
inline float dot(const vecf<D> &v1, const vecf<D> &v2);

template<unsigned char D>
inline void operator+=(vecf<D> &v1, const vecf<D> &v2) {
  for (unsigned char i = 0; i < D; i++) {
    v1[i] += v2[i];
  }
}

template<unsigned char D>
inline void operator-=(vecf<D> &v1, const vecf<D> &v2) {
  for (unsigned char i = 0; i < D; i++) {
    v1[i] -= v2[i];
  }
}

template<unsigned char D>
inline void operator*=(vecf<D> &v, float s) {
  for (unsigned char i = 0; i < D; i++) {
    v[i] *= s;
  }
}

template<unsigned char D>
inline void operator/=(vecf<D> &v, float s) {
  assert(s != 0);
  for (unsigned char i = 0; i < D; i++) {
    v[i] /= s;
  }
}

template<unsigned char D>
inline vecf<D> operator-(const vecf<D> &v) {
  vecf<D> out;
  for (unsigned char i = 0; i < D; i++) {
    out[i] = -v[i];
  }
  return out;
}

template<unsigned char D>
inline vecf<D> operator+(const vecf<D> &v1, const vecf<D> &v2) {
  vecf<D> out;
  for (unsigned char i = 0; i < D; i++) {
    out[i] = v1[i] + v2[i];
  }
  return out;
}

template<unsigned char D>
inline vecf<D> operator-(const vecf<D> &v1, const vecf<D> &v2) {
  vecf<D> out;
  for (unsigned char i = 0; i < D; i++) {
    out[i] = v1[i] - v2[i];
  }
  return out;
}

template<unsigned char D>
inline vecf<D> operator*(const vecf<D> &v, float s) {
  vecf<D> out;
  for (unsigned char i = 0; i < D; i++) {
    out[i] = v[i] * s;
  }
  return out;
}

template<unsigned char D>
inline vecf<D> operator/(const vecf<D> &v, float s) {
  assert(s != 0);
  vecf<D> out;
  for (unsigned char i = 0; i < D; i++) {
    out[i] = v[i] / s;
  }
  return out;
}

// for printing
template<typename T, unsigned char D>
T &operator<<(T &os, const vecf<D> &p) {
  os << "{";
  for (unsigned char i = 0; i < D; i++) {
    os << p[i];
    if (i != D - 1) {
      os << ",";
    }
  }
  os << "}";
  return os;
}

// matf operators

template<unsigned char R, unsigned char C>
inline void operator+=(matf<R, C> &m1, const matf<R, C> m2) {
  for (unsigned char i = 0; i < R; i++) {
    m1[i] += m2[i];
  }
}

template<unsigned char R, unsigned char C>
inline void operator-=(matf<R, C> &m1, const matf<R, C> m2) {
  for (unsigned char i = 0; i < R; i++) {
    m1[i] -= m2[i];
  }
}

template<unsigned char R, unsigned char C>
inline void operator*=(matf<R, C> &m, float s) {
  for (unsigned char i = 0; i < R; i++) {
    m[i] *= s;
  }
}

template<unsigned char R, unsigned char C>
inline void operator/=(matf<R, C> &m, float s) {
  assert(s != 0);
  for (unsigned char i = 0; i < R; i++) {
    m[i] /= s;
  }
}

template<unsigned char R, unsigned char C>
inline matf<R, C> operator-(const matf<R, C> &m) {
  matf<R, C> out;
  for (unsigned char i = 0; i < R; i++) {
    out[i] = -m[i];
  }
  return out;
}

template<unsigned char R, unsigned char C>
inline matf<R, C> operator+(const matf<R, C> &m1, const matf<R, C> &m2) {
  matf<R, C> out;
  for (unsigned char i = 0; i < R; i++) {
    out[i] = m1[i] + m2[i];
  }
  return out;
}

template<unsigned char R, unsigned char C>
inline matf<R, C> operator-(const matf<R, C> &m1, const matf<R, C> &m2) {
  matf<R, C> out;
  for (unsigned char i = 0; i < R; i++) {
    out[i] = m1[i] - m2[i];
  }
  return out;
}

template<unsigned char R, unsigned char C>
inline matf<R, C> operator*(const matf<R, C> &m, float s) {
  matf<R, C> out;
  for (unsigned char r = 0; r < R; r++) {
    out[r] = m[r] * s;
  }
  return out;
}

template<unsigned char R, unsigned char C>
inline matf<R, C> operator/(const matf<R, C> &m, float s) {
  assert(s != 0);
  matf<R, C> out;
  for (unsigned char r = 0; r < R; r++) {
    out[r] = m[r] / s;
  }
  return out;
}

template<unsigned char R, unsigned char C>
inline vecf<R> operator*(const matf<R, C> &m, const vecf<C> &v) {
  vecf<R> out;
  for (unsigned char r = 0; r < R; r++) {
    out[r] = dot(v, m[r]);
  }
  return out;
}

template<unsigned char A, unsigned char N, unsigned char B>
inline matf<A, B> operator*(const matf<A, N> &m1, const matf<N, B> &m2) {
  matf<A, B> out;
  for (unsigned char i = 0; i < A; i++) {
    for (unsigned char j = 0; j < B; j++) {
      float dot = m1[i][0] * m2[0][j];
      for (unsigned char k = 1; k < N; k++) {
        dot += m1[i][k] * m2[k][j];
      }
      out[i][j] = dot;
    }
  }
  return out;
}

template<typename T, unsigned char R, unsigned char C>
T &operator<<(T &os, const matf<R, C> &p) {
  os << "{";
  for (unsigned char d = 0; d < R; d++) {
    os << p[d];
    if (d != R - 1) {
      os << ",";
    }
  }
  os << "}";
  return os;
}

template<unsigned char D>
inline bool operator==(const vecf<D> &left, const vecf<D> right) {
  for (unsigned char i = 0; i < D; i++) {
    if (left[i] != right[i]) {
      return false;
    }
  }
  return true;
}

template<unsigned char D>
inline bool operator!=(const vecf<D> &left, const vecf<D> right) {
  for (unsigned char i = 0; i < D; i++) {
    if (left[i] != right[i]) {
      return true;
    }
  }
  return false;
}

// Math functions

inline float sign(float x) { return x < 0 ? -1.0f : x > 0 ? 1.0f : 0.0f; }

inline float min(float a, float b) { return a < b ? a : b; }

inline float max(float a, float b) { return a > b ? a : b; }

inline float clamp(float x, float a, float b) { return max(a, min(b, x)); }

template<unsigned char D>
inline float dot(const vecf<D> &v1, const vecf<D> &v2) {
  float accum = v1[0] * v2[0];
  for (unsigned char i = 1; i < D; i++) {
    accum += v1[i] * v2[i];
  }
  return accum;
};

template<unsigned char D>
inline matf<D, D> outer(const vecf<D> &v1, const vecf<D> &v2) {
  matf<D, D> out;
  for (unsigned i = 0; i < D; i++) {
    out[i] = v2 * v1[i];
  }
  return out;
}

// multiplies as if diag_m was a diagonal matrix on the left side
template<unsigned char D>
inline void scale_left(vecf<D> &v, const vecf<D> &diag_m) {
  for (unsigned char i = 0; i < D; i++) {
    v[i] *= diag_m[i];
  }
}

// multiplies as if diag_m was a diagonal matrix on the left side
template<unsigned char R, unsigned char C>
inline void scale_left(matf<R, C> &m, const vecf<C> &diag_m) {
  for (unsigned char i = 0; i < R; i++) {
    m[i] *= diag_m[i];
  }
}

template<unsigned char D>
inline void diag_sum(matf<D, D> &m, const vecf<D> &dm) {
  for (unsigned char i = 0; i < D; i++) {
    m[i][i] += dm[i];
  }
}

template<unsigned char D>
inline void diag_sub(matf<D, D> &m, const vecf<D> &dm) {
  for (unsigned char i = 0; i < D; i++) {
    m[i][i] -= dm[i];
  }
}

template<unsigned char D>
inline float length_squared(vecf<D> v) {
  return dot(v, v);
}

template<unsigned char D>
inline float length(vecf<D> v) {
  return sqrtf(dot(v, v));
}

template<unsigned char R, unsigned char C>
inline matf<C, R> transpose(const matf<R, C> m) {
  matf<C, R> out;
  for (unsigned char i = 0; i < R; i++) {
    for (unsigned char j = 0; j < C; j++) {
      out[j][i] = m[i][j];
    }
  }
  return out;
}

inline float cross(vec2f a, vec2f b) { return a.x * b.y - a.y * b.x; }

inline vec2f cross(float s, vec2f v) { return vec2f{-s * v.y, s * v.x}; }

inline vec2f cross(vec2f v, float s) { return vec2f{s * v.y, -s * v.x}; }

inline vec2f abs(vec2f v) { return {fabsf(v.x), fabsf(v.y)}; }

inline mat2f abs(mat2f a) { return {abs(a[0]), abs(a[1])}; }

inline mat2f rotation(float angle) {
  float c = cos(angle);
  float s = sin(angle);
  return mat2f{c, -s, s, c};
}

inline vec2f rotate(vec2f v, float rad) {
  float sin_value = sin(rad);
  float cos_value = cos(rad);

  vec2f rotated_v;
  rotated_v.x = v.x * cos_value - v.y * sin_value;
  rotated_v.y = v.x * sin_value + v.y * cos_value;

  return rotated_v;
}

inline vec2f transform(vec3f q, vec2f v) {
  v = rotate(v, q.z);
  v += q.xy();
  return v;
}

inline vec2f inv_transform(vec3f q, vec2f v) {
  v -= q.xy();
  v = rotate(v, -q.z);
  return v;
}

inline vec2f perp_l(vec2f v) { return vec2f{-v.y, v.x}; }

inline vec2f perp_r(vec2f v) { return vec2f{v.y, -v.x}; }

template<unsigned char D>
inline matf<D, D> identity() {
  matf<D, D> m;
  for (unsigned i = 0; i < D; i++) {
    for (unsigned j = 0; j < D; j++) {
      m[i][j] = 0;
    }
  }

  for (unsigned i = 0; i < D; i++) {
    m[i][i] = 1;
  }
  return m;
}

template<unsigned char D1, unsigned char D2>
inline vecf<D1 + D2> concat(const vecf<D1> &v1, const vecf<D2> &v2) {
  vecf<D1 + D2> out;
  for (unsigned i = 0; i < D1; i++) {
    out[i] = v1[i];
  }
  for (unsigned i = 0; i < D2; i++) {
    out[D1 + i] = v2[i];
  }
  return out;
};

inline vec3f solve(const mat3f &a, const vec3f &b) {
  // Compute LDL^T decomposition
  float D1 = a[0][0];
  float L21 = a[1][0] / a[0][0];
  float L31 = a[2][0] / a[0][0];
  float D2 = a[1][1] - L21 * L21 * D1;
  float L32 = (a[2][1] - L21 * L31 * D1) / D2;
  float D3 = a[2][2] - (L31 * L31 * D1 + L32 * L32 * D2);

  // Forward substitution: Solve Ly = b
  float y1 = b.x;
  float y2 = b.y - L21 * y1;
  float y3 = b.z - L31 * y1 - L32 * y2;

  // Diagonal solve: Solve Dz = y
  float z1 = y1 / D1;
  float z2 = y2 / D2;
  float z3 = y3 / D3;

  // Backward substitution: Solve L^T x = z
  vec3f x;
  x[2] = z3;
  x[1] = z2 - L32 * x[2];
  x[0] = z1 - L21 * x[1] - L31 * x[2];
  return x;
}

inline vec2f solve(const mat2f &a, const vec2f &b) {
  float d1 = a[0][0];
  float l21 = a[1][0] / d1;            // L_{21}
  float d2 = a[1][1] - l21 * l21 * d1; // D_2

  constexpr float eps = 1e-12f;
  assert(std::fabs(d1) > eps && std::fabs(d2) > eps);

  float y1 = b.x;
  float y2 = b.y - l21 * y1;

  float z1 = y1 / d1;
  float z2 = y2 / d2;

  vec2f x;
  x[1] = z2;
  x[0] = z1 - l21 * x[1];

  return x;
}
