#pragma once

namespace cp6 {
template<typename QD_T>
struct color_data_type;

template<typename QD_T>
using color_data_t = color_data_type<QD_T>::type;

#define SET_COLOR_DATA_T(qd_t, color_data_t)                                                       \
  template<>                                                                                       \
  struct color_data_type<qd_t> {                                                                   \
    using type = color_data_t;                                                                     \
  };

// coloring
template<typename T>
struct is_cqt {
  static constexpr bool value = requires { requires(T::colorable); };
};

struct flag_colorable {
  static constexpr bool colorable = true;
};
} // namespace cp6

template<typename COLOR_DATA_T>
void update_color_data(COLOR_DATA_T &color_data, const unsigned *to_q, unsigned cq_num);
