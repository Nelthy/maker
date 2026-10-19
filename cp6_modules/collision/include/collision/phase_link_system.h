#pragma once
#include "collision/pcol_state.h"
#include "collision_combo_decl.h"
#include "collision_flags.h"
#include "ds_utils.h"
#include "god_state.h"
#include "region_state.h"
#include "task_marker.h"
#include "tri_tuple.h"
#include "tri_utils.h"
#include "tuple_utils.h"
#include <iostream>
#include <type_traits>

namespace cp6 {
template<typename PCOLS, typename PHASE_LINKS>
void cull_pcols(PCOLS &pcols, const PHASE_LINKS &phase_links);

template<typename TW>
struct phase_link_system_tt {
  // static constexpr unsigned qt_num = get_qt_num<TW>();
  static constexpr unsigned plqt_num = get_matching_num<is_plqt, qt_ts<TW>>();
  static constexpr auto plqt_to_qt = generate_tuple_indirection<
    get_matching_ts<is_plqt, qt_ts<TW>>,
    qt_ts<TW>
  >();
  static constexpr auto qt_to_plqt = generate_tuple_indirection<
    qt_ts<TW>,
    get_matching_ts<is_plqt, qt_ts<TW>>
  >();

  static_assert(plqt_num > 0);

  template<unsigned QT1, unsigned QT2>
  static constexpr bool can_qts_collide() {
    return can_qts_collide_combo_v<qt_t<QT1, TW>, qt_t<QT2, TW>>;
  }

  // can be called before/after potential collisions are generated.
  void update_pt1(unsigned reg) {
    // clear the phase links
    tri_apply_spin<plqt_num>([&]<size_t PLQT1, size_t PLQT2> {
      static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
      static constexpr unsigned qt2 = plqt_to_qt[PLQT2];

      if constexpr (can_qts_collide<qt1, qt2>()) {
        return phase_links<PLQT1, PLQT2>(reg).clear();
      } else {
        return true;
      }
    });

    // resize the phase link storage
    const auto &reg_s = get<is_region_state>(*m_state);
    tri_apply_spin<plqt_num>([&]<size_t PLQT1, size_t PLQT2> {
      static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
      static constexpr unsigned qt2 = plqt_to_qt[PLQT2];

      if constexpr (can_qts_collide<qt1, qt2>()) {
        if constexpr (PLQT1 == PLQT2) {
          return phase_links<PLQT1, PLQT2>(reg).resize(
            dlm::size<qt1>(reg_s, reg)
          );
        } else {
          return phase_links<PLQT1, PLQT2>(reg).resize(
            dlm::size<qt1>(reg_s, reg),
            dlm::size<qt2>(reg_s, reg)
          );
        }
      } else {
        return true;
      }
    });

    // calling phase linkers
    static_assert(
      get_matching_num<is_phase_linker, full_tuple_t<TW>>(),
      "must be at least one phase linker in the cross state"
    );

    const auto phase_linkers = get_matching<is_phase_linker>(*m_state);
    static constexpr unsigned
      phase_linker_num = get_matching_num<is_phase_linker, full_tuple_t<TW>>();
    task_marker<tri_num(plqt_num)> tasks[phase_linker_num];

    spin_loop_i<phase_linker_num>([&]<size_t PL>() {
      auto &phase_linker = get<PL>(phase_linkers);
      tri_apply<plqt_num>([&]<size_t PLQT1, size_t PLQT2> {
        static constexpr unsigned t = tri_linearize(PLQT1, PLQT2, plqt_num);
        static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
        static constexpr unsigned qt2 = plqt_to_qt[PLQT2];

        if constexpr (can_qts_collide<qt1, qt2>()) {
          if (!tasks[PL].done(t)) {
            bool done = phase_linker.template add_phase_links<qt1, qt2>(
              reg,
              phase_links<PLQT1, PLQT2>(reg)
            );
            if (done) { tasks[PL].mark_as_done(t); }
          }
        }
      });
      return tasks[PL].all_done();
    });
  }

  // must be called after potential collisions are generated
  void update_pt2(unsigned reg) {
    // null potential collisions that violate a phase links
    tri_apply_spin<plqt_num>([&]<unsigned PLQT1, unsigned PLQT2> {
      static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
      static constexpr unsigned qt2 = plqt_to_qt[PLQT2];
      auto &pcols = get<pcol_state>(*m_state).template get_pcols<qt1, qt2>(reg);

      return cull_pcols(pcols, phase_links<PLQT1, PLQT2>(reg));
    });
  };

  void update(unsigned reg) {
    update_pt1(reg);
    update_pt2(reg);
  }

  friend struct god::state<TW>;

  template<unsigned QT1, unsigned QT2>
  const auto &get_phase_links(unsigned reg) const {
    static constexpr unsigned plqt1 = qt_to_plqt[QT1];
    static constexpr unsigned plqt2 = qt_to_plqt[QT2];
    static_assert(plqt1 != -1 && plqt2 != -1, "both qts must be phaselinkable");

    return phase_links<plqt1, plqt2>(reg);
  }

private:
  template<unsigned PLQT1, unsigned PLQT2>
  auto &phase_links(unsigned reg) {
    static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
    static constexpr unsigned qt2 = plqt_to_qt[PLQT2];
    static_assert(can_qts_collide<qt1, qt2>());

    return m_reg_data.phase_links()[reg].template get<PLQT1, PLQT2>();
  }

  template<unsigned PLQT1, unsigned PLQT2>
  const auto &phase_links(unsigned reg) const {
    static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
    static constexpr unsigned qt2 = plqt_to_qt[PLQT2];
    static_assert(can_qts_collide<qt1, qt2>());

    return m_reg_data.phase_links()[reg].template get<PLQT1, PLQT2>();
  }

  void init(god::state<TW> &state) {
    m_state = &state;
    auto &regions = get<is_region_state>(state);
    regions.add_r1_external(m_reg_data);
  }

  template<unsigned PLQT1, unsigned PLQT2>
  struct qt_tri_gen {
    static constexpr unsigned qt1 = plqt_to_qt[PLQT1];
    static constexpr unsigned qt2 = plqt_to_qt[PLQT2];
    using type = std::conditional_t<
      qt1 == qt2,
      phase_links_self_t<cd_t<qt1, TW>>,
      phase_links_combo_t<cd_t<qt1, TW>, cd_t<qt2, TW>>
    >;
  };

  struct reg_data_set:
  dlm::ext<dlm::perm<
    dlm::vec_ds<tri_tuple<plqt_num, qt_tri_gen>> // [reg]<plqt1, plqt2>
  >> {
    DEFINE_DS_FIELD((0), phase_links);
  } m_reg_data;

  god::state<TW> *m_state;
};

struct phase_link_system {
  template<typename TW>
  using template_t = phase_link_system_tt<TW>;
};
} // namespace cp6

/*
namespace cp6 {
template<typename PCOLS, typename PHASE_LINKS>
void cull_pcols(PCOLS &pcols, const PHASE_LINKS &phase_links);

template<typename... Ts>
struct phase_link_system_tt {
  static constexpr unsigned qt_num = get_qt_num<TW>();
  static constexpr unsigned plqt_num = get_matching_num<is_plqt, qt_ts<TW>>();
  static_assert(plqt_num > 0);

  template<unsigned QT1, unsigned QT2>
  static constexpr bool can_qts_collide() {
    return can_qts_collide_combo_v<qt_t<QT1, TW>, qt_t<QT2, TW>>;
  }

  // can be called before/after potential collisions are generated.
  void update_pt1(unsigned reg) {
    // clear the phase links
    tri_apply_spin<qt_num>([&]<size_t QT1, size_t QT2> {
      if constexpr (is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value) { if constexpr (can_qts_collide<QT1, QT2>()) { return
get_phase_links<QT1, QT2>(reg).clear(); } else { return true;
        }
      }
    });

    // resize the phase link storage
    const auto &reg_s = get<is_region_state>(*m_state);
    tri_apply_spin<qt_num>([&]<size_t QT1, size_t QT2> {
      if constexpr (is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value) { if constexpr (can_qts_collide<QT1, QT2>()) { if constexpr (QT1 ==
QT2) { return get_phase_links<QT1, QT2>(reg).resize(dlm::size<QT1>(reg_s, reg));
          } else {
            return get_phase_links<QT1, QT2>(reg).resize(
              dlm::size<QT1>(reg_s, reg),
              dlm::size<QT2>(reg_s, reg)
            );
          }
        } else {
          return true;
        }
      }
    });

    // calling phase linkers
    static_assert(
      get_matching_num<is_phase_linker, full_tuple_t<TW>>(),
      "must be at least one phase linker in the cross state"
    );

    const auto phase_linkers = get_matching<is_phase_linker>(*m_state);
    tuple_spin_loop(
      [&](auto &phase_linker) {
        tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
          if constexpr (is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value) { if constexpr (can_qts_collide<QT1, QT2>()) { return
phase_linker.template add_phase_links<QT1, QT2>( reg, get_phase_links<QT1,
QT2>(reg)
              );
            }
          }
        });
      },
      phase_linkers
    );
  }

  // must be called after potential collisions are generated
  void update_pt2(unsigned reg) {
    // null potential collisions that violate a phase links
    tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
      if constexpr (is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value) {

        auto &pcols = get<pcol_state>(*m_state).template get_pcols<QT1,
QT2>(reg); const auto &phase_links = get_phase_links<QT1, QT2>(reg);
        cull_pcols(pcols, phase_links);
      }
    });
  };

  void update(unsigned reg) {
    update_pt1(reg);
    update_pt2(reg);
  }

  friend struct cross_state<TW>;

  template<unsigned QT1, unsigned QT2>
  auto &get_phase_links(unsigned reg) {
    static_assert(is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value); static_assert(can_qts_collide_combo_v<qt_t<QT1, TW>, qt_t<QT1,
TW>>);

    return m_reg_data.phase_links()[reg].template get<QT1, QT2>();
  }

  template<unsigned QT1, unsigned QT2>
  const auto &get_phase_links(unsigned reg) const {
    static_assert(is_plqt<qt_t<QT1, TW>>::value && is_plqt<qt_t<QT2,
TW>>::value); static_assert(can_qts_collide_combo_v<qt_t<QT1, TW>, qt_t<QT1,
TW>>);

    return m_reg_data.phase_links()[reg].template get<QT1, QT2>();
  }

private:
  void init(cross_state<TW> &state) {
    m_state = &state;
    auto &regions = get<is_region_state>(state);
    regions.add_r1_external(m_reg_data);
  }

  template<unsigned QT1, unsigned QT2>
  struct qt_tri_gen {
    using type = std::conditional_t<
      QT1 == QT2,
      phase_links_self_t<cd_t<QT1, TW>>,
      phase_links_combo_t<cd_t<QT1, TW>, cd_t<QT2, TW>>
    >;
  };

  struct reg_data_set:
  dlm::ext<dlm::perm<
    dlm::vec_ds<tri_tuple<qt_num, qt_tri_gen>> // [reg]<qt1, qt2>
  >> {
    DEFINE_DS_FIELD((0), phase_links);
  } m_reg_data;

  cross_state<TW> *m_state;
};

struct phase_link_system {
  template<typename... Ts>
  using template_t = phase_link_system_tt<TW>;
};
} // namespace cp6
 */
