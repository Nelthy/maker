#pragma once

namespace cp6 {
struct vbd_flags {
  bool immovable : 1;
  bool dont_resolve_collisions : 1;
  bool reserved : 6;
};

// fg = force generator
template<typename T>
struct is_avbd_jacobi_fg {
  static constexpr bool value = requires { requires(T::avbd_jacobi_fg); };
};

struct flag_avbd_jacobi_fg {
  static constexpr bool avbd_jacobi_fg = true;
};

// gs = Gauss Seidel
template<typename T>
struct is_avbd_gs_fg {
  static constexpr bool value = requires { requires(T::avbd_gs_fg); };
};

struct flag_avbd_gs_fg {
  static constexpr bool avbd_gs_fg = true;
};

template<typename T>
struct is_avbd_fg {
  static constexpr bool value = is_avbd_gs_fg<T>::value
                             || is_avbd_jacobi_fg<T>::value;
};

template<typename T>
struct is_vbd_qt {
  static constexpr bool value = requires { requires(T::vbd_qt); };
};

struct flag_vbd_qt {
  static constexpr bool vbd_qt = true;
};

template<typename QT1, typename QT2>
struct has_collision_force {
  static constexpr bool value = true;
};

template<typename QT1, typename QT2>
static constexpr bool
  has_collision_force_v = has_collision_force<QT1, QT2>::value;

#define DISABLE_COLLISION_FORCE(qt1, qt2)                                      \
  template<>                                                                   \
  struct cp6::has_collision_force<qt1, qt2> {                                  \
    static constexpr bool value = false;                                       \
  };

// pcnt

template<typename T>
struct uses_avbd_pcnt {
  static constexpr bool value = requires { T::avbd_pcnt; };
};

struct flag_avbd_pcnt {
  static constexpr bool avbd_pcnt = true;
};

template<typename T>
struct uses_gs_for_avbd_pcnt {
  static constexpr bool value = requires { requires(T::gs_for_avbd_pcnt); };
};

struct flag_gs_for_avbd_pcnt {
  static constexpr bool gs_for_avbd_pcnt = true;
};

// tcnt

template<typename T>
struct uses_avbd_tcnt_force {
  static constexpr bool value = requires { T::avbd_tcnt_force; };
};

struct flag_avbd_tcnt_force {
  static constexpr bool avbd_tcnt_force = true;
};

template<typename T>
struct uses_gs_for_avbd_tcnt {
  static constexpr bool value = requires { requires(T::gs_for_avbd_tcnt); };
};

struct flag_gs_for_avbd_tcnt {
  static constexpr bool gs_for_avbd_tcnt = true;
};

// pin

template<typename T>
struct is_pinqt {
  static constexpr bool value = requires { requires(T::avbd_pinnable); };
};

struct flag_avbd_pinnable {
  static constexpr bool avbd_pinnable = true;
};

// liquid
template<typename T>
struct is_vbd_lqt {
  static constexpr bool value = requires { requires(T::vbd_liquid); };
};

struct flag_vbd_liquid {
  static constexpr bool vbd_liquid = true;
};
} // namespace cp6
