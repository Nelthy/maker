#pragma once
#include "collision_combo_decl.h"
#include "god_state.h"
#include <region_state.h>
#include <tri_tuple.h>

namespace cp6 {

template<typename TW>
struct pcol_state_tt {
private:
  friend god::state<TW>;
  static constexpr unsigned qt_num = get_qt_num<TW>();

  template<unsigned QT1, unsigned QT2>
  struct pcol_tri_gen {
    using type = pcol_combo_t<cd_t<QT1, TW>, cd_t<QT2, TW>>;
  };

  void init(god::state<TW> &state) {
    auto &reg_s = get<is_region_state>(state);
    reg_s.add_r1_external(m_pcols);
  }

public:
  dlm::ext<dlm::perm< // [reg]
    dlm::vec_ds<tri_tuple<qt_num, pcol_tri_gen>>
  >>
    m_pcols;

  template<unsigned QT1, unsigned QT2>
  auto &get_pcols(unsigned reg) {
    return m_pcols.template get<0>()[reg].template get<QT1, QT2>();
  }

  template<unsigned QT1, unsigned QT2>
  const auto &get_pcols(unsigned reg) const {
    return m_pcols.template get<0>()[reg].template get<QT1, QT2>();
  }
};

struct pcol_state {
  template<typename... Ts>
  using template_t = pcol_state_tt<Ts...>;
};

} // namespace cp6
