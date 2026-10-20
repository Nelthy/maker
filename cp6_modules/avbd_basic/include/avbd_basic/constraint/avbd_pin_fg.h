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

template<typename TW>
struct avbd_pin_fg_tt: pair_force_fg_tt<is_pinqt, TW>, flag_avbd_gs_fg {
  using base = pair_force_fg_tt<is_pinqt, TW>;

  template<unsigned QT1, unsigned QT2>
  void add_pin(unsigned reg, unsigned q_a, unsigned q_b, pin_data data) {
    unsigned pf = base::template add_pf<QT1, QT2>(reg, q_a, q_b);
    base::template get_pf_state<QT1, QT2>(reg)[pf] = std::move(data);
  }

  template<unsigned QT1, unsigned QT2>
  void remove_pin(unsigned reg, unsigned pf) {
    base::template remove_pf<QT1, QT2>(reg, pf);
  }

  // template<unsigned QT1, unsigned QT2, typename T>
  // void add_phase_links(unsigned reg, T &phase_links) {
  //   assert(false);
  // }
};

struct avbd_pin_fg {
  template<typename TW>
  using template_t = avbd_pin_fg_tt<TW>;
};
} // namespace cp6
