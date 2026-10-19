#pragma once
#include "avbd/avbd_flags.h"
#include "maths.h"
#include "pair_force_fg.h"

namespace cp6 {

struct pin_data {
  vec2f ra = {0, 0};
  vec2f rb = {0, 0};
  vec2f lambda = {0, 0};
  vec2f k = {0, 0};
  vec2f C0 = {0, 0};
};

template<typename QD_A, typename QD_B>
struct pair_force_state<is_pinqt, QD_A, QD_B> {
  using type = dlm::vec_ds<pin_data>;
};

template<typename... Ts>
struct avbd_pin_fg_tt: pair_force_fg_tt<is_pinqt, Ts...>, flag_avbd_gs_fg {
  using base = pair_force_fg_tt<is_pinqt, Ts...>;

  template<unsigned QT_A, unsigned QT_B>
  void add_pin(unsigned reg, unsigned q_a, unsigned q_b, pin_data data) {
    unsigned pf = base::template add_pf<QT_A, QT_B>(reg, q_a, q_b);
    base::template get_pf_state<QT_A, QT_B>(reg)[pf] = std::move(data);
  }

  // template<unsigned QT1, unsigned QT2, typename T>
  // void add_phase_links(unsigned reg, T &phase_links) {
  //   assert(false);
  // }
};

struct avbd_pin_fg {
  template<typename... Ts>
  using template_t = avbd_pin_fg_tt<Ts...>;
};
} // namespace cp6
