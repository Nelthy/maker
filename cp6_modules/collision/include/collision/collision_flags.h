#pragma once

namespace cp6 {
template<typename T>
struct is_phase_linker {
  static constexpr bool value = requires { requires(T::phase_linker); };
};

struct flag_phase_linker {
  static constexpr bool phase_linker = true;
};

// plqt = phase linkable coordiante type
template<typename T>
struct is_plqt {
  static constexpr bool value = requires { requires(T::plqt); };
};

struct flag_plqt {
  static constexpr bool plqt = true;
};

// pcqt = persistent contact coordiante type
template<typename T>
struct is_pcqt {
  static constexpr bool value = requires { requires(T::pcqt); };
};

struct flag_pcqt {
  static constexpr bool pcqt = true;
};
}; // namespace cp6
