#pragma once
#include "ab_link.h"
#include "god_state.h"
#include "region_state.h"
#include <collision/pcol_state.h>
// user defined function
template<typename BD1_T, typename BD2_T>
inline bool overlapping(
  const BD1_T &bd1,
  const BD2_T &bd2,
  unsigned q1,
  unsigned q2
);

bool update_pcols(const auto &bd1, const auto &bd2, auto &data);

bool update_pcols_self(const auto &bd1, auto &data);

namespace cp6 {
template<typename TW>
struct sap_broadphase_tt {
  static constexpr char qt_num = get_qt_num<TW>();

  template<unsigned QT1, unsigned QT2>
  static constexpr bool can_qts_collide() {
    return can_qts_collide_combo_v<qt_t<QT1, TW>, qt_t<QT2, TW>>;
  }

  void update(unsigned reg) {
    auto &reg_s = get<is_region_state>(*m_state);
    auto &pcol_s = get<pcol_state>(*m_state);
    tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
      if (can_qts_collide<QT1, QT2>()) {
        // broadphase.
        auto &curr_pcols = pcol_s.template get_pcols<QT1, QT2>(reg);
        curr_pcols.clear();

        const auto &qt1 = reg_s.template get_qt<QT1>(reg);
        const auto &qt2 = reg_s.template get_qt<QT2>(reg);

        if constexpr (QT1 != QT2) {
          const auto &bd1 = reg_s.template get_qt<QT1>(reg).cd().bd();
          const auto &bd2 = reg_s.template get_qt<QT2>(reg).cd().bd();

          update_pcols(bd1, bd2, curr_pcols);
        } else {
          // self collisions are looped over slightly differently
          const auto &bd = reg_s.template get_qt<QT1>(reg).cd().bd();
          update_pcols_self(bd, curr_pcols);
        }
      }
    });
  }

private:
  friend struct god::state<TW>;

  void init(god::state<TW> &state) { m_state = &state; }

  god::state<TW> *m_state;
};

struct sap_broadphase {
  template<typename TW>
  using template_t = sap_broadphase_tt<TW>;
};
} // namespace cp6
