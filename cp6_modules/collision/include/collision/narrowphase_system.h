#pragma once
#include "collision_combo_decl.h"
#include "god_state.h"
#include "ds_modifier.h"
#include "ds_utils.h"
#include "pcol_state.h"
#include "region_state.h"
#include "tri_tuple.h"
#include "tri_utils.h"
#include <cassert>

namespace cp6 {
template<typename TW>
struct narrowphase_system_tt {
  static constexpr unsigned qt_num = get_qt_num<TW>();

  void update(unsigned reg) {
    auto &pcol_s = get<pcol_state>(*m_state);
    auto &reg_s = get<is_region_state>(*m_state);
    tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
      dlm::ds_modifier::clear(m_reg_data.tcnt_data()[reg].template get<QT1, QT2>(), dlm::ikey{});
    });

    tri_apply_spin<qt_num>([&]<size_t QT1, size_t QT2> {
      return narrowphase(
        reg_s.template get_qt<QT1>(reg).cd(),
        reg_s.template get_qt<QT2>(reg).cd(),
        pcol_s.template get_pcols<QT1, QT2>(reg),
        get_tcnt_data<QT1, QT2>(reg)
      );
    });

    // if the narrowphase functions are multithreaded then we wait for them to return here
    tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
      if constexpr (requires { get_tcnt_data<QT1, QT2>(reg).wait(); }) {
        get_tcnt_data<QT1, QT2>(reg).wait();
      }
    });
  }

  template<unsigned QT1, unsigned QT2>
  const auto &get_tcnt_data(unsigned reg) const {
    return m_reg_data.tcnt_data()[reg].template get<QT1, QT2>();
  }

  template<unsigned QT1, unsigned QT2>
  auto &get_tcnt_data(unsigned reg) {
    return m_reg_data.tcnt_data()[reg].template get<QT1, QT2>();
  }

private:
  friend struct god::state<TW>;

  void init(god::state<TW> &state) {
    m_state = &state;
    get<is_region_state>(state).add_r1_external(m_reg_data);
  };

  template<unsigned QT_A, unsigned QT_B>
  struct tcnt_data_tri_gen {
    using type = tcnt_combo_t<cd_t<QT_A, TW>, cd_t<QT_B, TW>>;
  };

  struct reg_data_set:
  dlm::ext<dlm::perm< // [reg]
    dlm::vec_ds<tri_tuple<qt_num, tcnt_data_tri_gen>>
  >> {
    DEFINE_DS_FIELD((0), tcnt_data);
  } m_reg_data;

  god::state<TW> *m_state;
};

struct narrowphase_system {
  template<typename TW>
  using template_t = narrowphase_system_tt<TW>;
};
} // namespace cp6
