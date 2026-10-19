#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/avbd_system.h"
#include "avbd/color_decl.h"
#include "avbd_cnt_decl.h"
#include "collision/pcnt_system.h"
#include "ds_modifier.h"
#include "god_state.h"
#include "perm_tri_tuple.h"
#include "region_state.h"
#include "size_ds.h"
#include "task_marker.h"
#include "tri_utils.h"
#include "tuple_utils.h"

namespace cp6 {
template<typename T>
struct uses_pcnt_jacobi_fg {
  static constexpr bool value = uses_avbd_pcnt<T>::value && !uses_gs_for_avbd_pcnt<T>::value;
};

template<typename TW>
struct avbd_pcnt_fg_tt: flag_avbd_jacobi_fg, flag_avbd_gs_fg {

  static constexpr auto vbd_pcqt_to_qt =
    generate_tuple_indirection<get_matching_ts<uses_avbd_pcnt, qt_ts<TW>>, qt_ts<TW>>();

  static constexpr auto qt_to_vbd_pcqt =
    generate_tuple_indirection<qt_ts<TW>, get_matching_ts<uses_avbd_pcnt, qt_ts<TW>>>();

  static constexpr auto vbd_qt_to_qt =
    generate_tuple_indirection<get_matching_ts<is_vbd_qt, qt_ts<TW>>, qt_ts<TW>>();

  static constexpr auto vbd_qt_to_vbd_pcqt = generate_tuple_indirection<
    get_matching_ts<is_vbd_qt, qt_ts<TW>>,
    get_matching_ts<uses_avbd_pcnt, qt_ts<TW>>
  >();
  static constexpr unsigned vbd_pcqt_num = get_matching_num<uses_avbd_pcnt, qt_ts<TW>>();

  static_assert(vbd_pcqt_num > 0);
  static constexpr auto vbd_pcqt_to_pcqt = generate_tuple_indirection<
    get_matching_ts<uses_avbd_pcnt, qt_ts<TW>>,
    get_matching_ts<is_pcqt, qt_ts<TW>>
  >();

  template<unsigned QT1, unsigned QT2>
  static constexpr bool has_collision_force() {
    return has_collision_force_v<qt_t<QT1, TW>, qt_t<QT2, TW>>;
  }

  template<unsigned QT>
  static constexpr bool uses_pcnt() {
    return uses_avbd_pcnt<qt_t<QT, TW>>::value;
  };

  template<unsigned QT1, unsigned QT2>
  auto &get_avbd_pcnt_data(unsigned reg) {
    static constexpr unsigned vbd_pcnt_qt1 = qt_to_vbd_pcqt[QT1];
    static constexpr unsigned vbd_pcnt_qt2 = qt_to_vbd_pcqt[QT2];
    static_assert(vbd_pcnt_qt1 != -1 && vbd_pcnt_qt2 != -1);
    static_assert(has_collision_force_v<qt_t<QT1, TW>, qt_t<QT2, TW>>);
    return m_avbd_pcnt_set.template avbd_pcnt_data<vbd_pcnt_qt1, vbd_pcnt_qt2>()[reg];
  }

  template<unsigned QT1, unsigned QT2>
  auto &get_q1_to_pcnt(unsigned reg) {
    static constexpr unsigned vbd_pcnt_qt1 = qt_to_vbd_pcqt[QT1];
    static constexpr unsigned vbd_pcnt_qt2 = qt_to_vbd_pcqt[QT2];
    static_assert(vbd_pcnt_qt1 != -1 && vbd_pcnt_qt2 != -1);
    static_assert(has_collision_force_v<qt_t<QT1, TW>, qt_t<QT2, TW>>);
    static_assert(uses_gs_for_avbd_pcnt<qt_t<QT1, TW>>::value);
    static_assert(uses_gs_for_avbd_pcnt<qt_t<QT2, TW>>::value);
    return m_avbd_pcnt_set.template q1_to_pcnt<vbd_pcnt_qt1, vbd_pcnt_qt2>()[reg];
  }

  template<unsigned QT1, unsigned QT2>
  auto &get_q2_to_pcnt(unsigned reg) {
    static constexpr unsigned vbd_pcnt_qt1 = qt_to_vbd_pcqt[QT1];
    static constexpr unsigned vbd_pcnt_qt2 = qt_to_vbd_pcqt[QT2];
    static_assert(vbd_pcnt_qt1 != -1 && vbd_pcnt_qt2 != -1);
    static_assert(has_collision_force_v<qt_t<QT1, TW>, qt_t<QT2, TW>>);
    static_assert(uses_gs_for_avbd_pcnt<qt_t<QT1, TW>>::value);
    static_assert(uses_gs_for_avbd_pcnt<qt_t<QT2, TW>>::value);
    return m_avbd_pcnt_set.template q2_to_pcnt<vbd_pcnt_qt1, vbd_pcnt_qt2>()[reg];
  }

private:
  friend struct god::state<TW>;
  friend class avbd_system_tt<TW>;
  friend class pcnt_system_tt<TW>;

  task_marker<tri_num(vbd_pcqt_num)> tasks1;
  task_marker<tri_num(vbd_pcqt_num)> tasks2;
  task_marker<tri_num(vbd_pcqt_num)> tasks3;

  bool start_step(unsigned reg, float h) {
    assert(tasks1.all_done());
    assert(tasks2.all_done());
    assert(tasks3.all_done());

    auto &reg_s = get<is_region_state>(*m_state);
    auto &pcnt_s = get<pcnt_system>(*m_state);

    tri_apply<vbd_pcqt_num>([&]<size_t VBD_PCQT1, size_t VBD_PCQT2> {
      static constexpr unsigned qt1 = vbd_pcqt_to_qt[VBD_PCQT1];
      static constexpr unsigned qt2 = vbd_pcqt_to_qt[VBD_PCQT2];
      static constexpr unsigned t = tri_linearize(VBD_PCQT1, VBD_PCQT2, vbd_pcqt_num);

      if constexpr (has_collision_force<qt1, qt2>()) {
        auto &avbd_pcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);
        auto &pcnt_data = pcnt_s.template get_pcnt_data<qt1, qt2>(reg);
        auto &qd1 = reg_s.template get_qt<qt1>(reg).qd();
        auto &qd2 = reg_s.template get_qt<qt2>(reg).qd();

        if (!tasks1.done(t)) {
          if (init_avbd_state(qd1, qd2, pcnt_data, avbd_pcnt_data)) {
            tasks1.mark_as_done(t);
          }
        }
      }
    });

    if (!tasks1.all_done()) {
      assert(false);
      return false;
    };

    tri_apply<vbd_pcqt_num>([&]<size_t VBD_PCQT1, size_t VBD_PCQT2> {
      static constexpr unsigned qt1 = vbd_pcqt_to_qt[VBD_PCQT1];
      static constexpr unsigned qt2 = vbd_pcqt_to_qt[VBD_PCQT2];
      static constexpr unsigned t = tri_linearize(VBD_PCQT1, VBD_PCQT2, vbd_pcqt_num);

      if constexpr (has_collision_force<qt1, qt2>()) {
        auto &qd1 = reg_s.template get_qt<qt1>(reg).qd();
        auto &qd2 = reg_s.template get_qt<qt2>(reg).qd();
        const auto &avbd_pcnt_datac = get_avbd_pcnt_data<qt1, qt2>(reg);

        if constexpr (uses_gs_for_avbd_pcnt<qt_t<qt1, TW>>::value) {
          if (!tasks2.done(t)) {
            if (update_q1_to_cnt(avbd_pcnt_datac, get_q1_to_pcnt<qt1, qt2>(reg), qd1.size())) {
              tasks2.mark_as_done(t);
            }
          }
        }

        if constexpr (uses_gs_for_avbd_pcnt<qt_t<qt1, TW>>::value) {
          if (!tasks3.done(t)) {
            if (update_q2_to_cnt(avbd_pcnt_datac, get_q2_to_pcnt<qt1, qt2>(reg), qd2.size())) {
              tasks3.mark_as_done(t);
            }
          }
        }
      }
    });

    if (tasks2.all_done() && tasks3.all_done()) {
      tasks1.reset();
      tasks2.reset();
      tasks3.reset();
      return true;
    } else {
      // assert(false);
      return false;
    }
  };

  bool add_derivs(unsigned reg) {
    assert(tasks1.all_done());
    assert(tasks2.all_done());
    assert(tasks3.all_done());

    for_each_i<vbd_pcqt_num>([&]<size_t vbd_pcqt> {
      static constexpr unsigned vbd_qt = vbd_pcqt_to_qt[vbd_pcqt];
      static constexpr unsigned qt = vbd_qt_to_qt[vbd_qt];

      if constexpr (!uses_gs_for_avbd_pcnt<qt_t<qt, TW>>::value) {
        auto &np_s = get<narrowphase_system>(*m_state);
        auto &reg_s = get<is_region_state>(*m_state);

        for_each_i<vbd_pcqt_num - vbd_pcqt>([&]<size_t I> {
          static constexpr unsigned vbd_pcqt1 = vbd_pcqt;
          static constexpr unsigned vbd_pcqt2 = vbd_pcqt + I;
          static constexpr unsigned qt1 = vbd_pcqt_to_qt[vbd_pcqt1];
          static constexpr unsigned qt2 = vbd_pcqt_to_qt[vbd_pcqt2];
          static constexpr unsigned t = tri_linearize(vbd_pcqt1, vbd_pcqt2, vbd_pcqt_num);

          if constexpr ((uses_pcnt<qt1>() || uses_pcnt<qt2>()) && has_collision_force<qt1, qt2>()) {
            auto &qd1 = reg_s.template get_qt<qt1>(reg).qd();
            auto &qd2 = reg_s.template get_qt<qt2>(reg).qd();
            auto &avbd_pcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);

            if (!tasks1.done(t)) {
              if (add_derivs1(qd1, qd2, avbd_pcnt_data)) {
                tasks1.mark_as_done(t);
              }
            }
          }
        });

        for_each_i<vbd_pcqt + 1>([&]<size_t OTHER_VBD_PCQT> {
          static constexpr unsigned vbd_pcqt1 = OTHER_VBD_PCQT;
          static constexpr unsigned vbd_pcqt2 = vbd_pcqt;
          static constexpr unsigned qt1 = vbd_pcqt_to_qt[vbd_pcqt1];
          static constexpr unsigned qt2 = vbd_pcqt_to_qt[vbd_pcqt2];
          static constexpr unsigned t = tri_linearize(vbd_pcqt1, vbd_pcqt2, vbd_pcqt_num);

          if constexpr ((uses_pcnt<qt1>() || uses_pcnt<qt2>()) && has_collision_force<qt1, qt2>()) {
            auto &qd1 = reg_s.template get_qt<qt1>(reg).qd();
            auto &qd2 = reg_s.template get_qt<qt2>(reg).qd();
            auto &avbd_pcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);

            if (!tasks2.done(t)) {
              if (add_derivs2(qd1, qd2, avbd_pcnt_data)) {
                tasks2.mark_as_done(t);
              }
            }
          }
        });
      }
    });

    if (tasks1.all_done() && tasks2.all_done()) {
      tasks1.reset();
      tasks2.reset();
      return true;
    } else {
      assert(false);
      return false;
    }
  }

  bool add_derivs(unsigned reg, unsigned c) {
    assert(tasks1.all_done());
    assert(tasks2.all_done());
    assert(tasks3.all_done());

    for_each_i<vbd_pcqt_num>([&]<size_t VBD_PCQT> {
      static constexpr unsigned qt = vbd_pcqt_to_qt[VBD_PCQT];
      if constexpr (uses_gs_for_avbd_pcnt<qt_t<qt, TW>>::value) {
        auto &reg_s = get<is_region_state>(*m_state);
        auto &color_s = get<color_system>(*m_state);

        for_each_i<vbd_pcqt_num - VBD_PCQT>([&]<size_t I> {
          static constexpr unsigned vbd_pcqt1 = VBD_PCQT;
          static constexpr unsigned vbd_pcqt2 = VBD_PCQT + I;
          static constexpr unsigned qt1 = vbd_pcqt_to_qt[vbd_pcqt1];
          static constexpr unsigned qt2 = vbd_pcqt_to_qt[vbd_pcqt2];
          static constexpr unsigned t = tri_linearize(vbd_pcqt1, vbd_pcqt2, vbd_pcqt_num);

          static_assert(is_cqt<qt_t<qt1, TW>>::value, "qt must be colorable to use gauss siedel");

          if constexpr (has_collision_force<qt1, qt2>()) {
            auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
            const auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
            const auto &avbd_pcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);
            const auto &q1_to_pcnt = get_q1_to_pcnt<qt1, qt2>(reg);
            const auto &color_data = color_s.template get_color_data<qt1>(reg, c);

            if (!tasks1.done(t)) {
              if (add_derivs1(qd1_s, qd2_s, avbd_pcnt_data, color_data, q1_to_pcnt)) {
                tasks1.mark_as_done(t);
              }
            }
          }
        });

        for_each_i<VBD_PCQT + 1>([&]<size_t OTHER_VBD_PCQT> {
          static constexpr unsigned vbd_pcqt1 = OTHER_VBD_PCQT;
          static constexpr unsigned vbd_pcqt2 = VBD_PCQT;
          static constexpr unsigned qt1 = vbd_pcqt_to_qt[vbd_pcqt1];
          static constexpr unsigned qt2 = vbd_pcqt_to_qt[vbd_pcqt2];
          static constexpr unsigned t = tri_linearize(vbd_pcqt1, vbd_pcqt2, vbd_pcqt_num);

          static_assert(is_cqt<qt_t<qt2, TW>>::value, "qt must be colorable to use gauss siedel");

          if constexpr (has_collision_force<qt1, qt2>()) {
            const auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
            auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
            const auto &avbd_pcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);
            const auto &q2_to_pcnt = get_q2_to_pcnt<qt1, qt2>(reg);
            const auto &color_data = color_s.template get_color_data<qt2>(reg, c);

            if (!tasks2.done(t)) {
              if (add_derivs2(qd1_s, qd2_s, avbd_pcnt_data, color_data, q2_to_pcnt)) {
                tasks2.mark_as_done(t);
              }
            }
          }
        });
      }
    });

    if (tasks1.all_done() && tasks2.all_done()) {
      tasks1.reset();
      tasks2.reset();
      return true;
    } else {
      assert(false);
      return false;
    }
  }

  bool after_step(unsigned reg, float h) {
    auto &np_system = get<narrowphase_system>(*m_state);
    auto &reg_s = get<is_region_state>(*m_state);

    tri_apply<vbd_pcqt_num>([&]<size_t VBD_PCQT1, size_t VBD_PCQT2> {
      static constexpr unsigned qt1 = vbd_pcqt_to_qt[VBD_PCQT1];
      static constexpr unsigned qt2 = vbd_pcqt_to_qt[VBD_PCQT2];
      static constexpr unsigned t = tri_linearize(VBD_PCQT1, VBD_PCQT2, vbd_pcqt_num);

      if constexpr ((uses_pcnt<qt1>() || uses_pcnt<qt2>()) && has_collision_force<qt1, qt2>()) {
        auto &avbd_tcnt_data = get_avbd_pcnt_data<qt1, qt2>(reg);
        auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
        auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
        if (!tasks1.done(t)) {
          if (update_avbd_vars(qd1_s, qd2_s, avbd_tcnt_data)) {
            tasks1.mark_as_done(t);
          };
        }
      }
    });

    if (tasks1.all_done()) {
      tasks1.reset();
      return true;
    } else {
      assert(false);
      return false;
    }
  }

  bool end_step(unsigned reg, float h) { return true; }

  void init(god::state<TW> &state) {
    m_state = &state;

    auto &region_state = get<is_region_state>(state);
    auto &pcnt_s = get<pcnt_system>(state);

    region_state.add_r1_external(m_avbd_pcnt_set);
    tri_apply<vbd_pcqt_num>([&]<size_t VBD_PCQT1, size_t VBD_PCQT2> {
      static constexpr unsigned qt1 = vbd_pcqt_to_qt[VBD_PCQT1];
      static constexpr unsigned qt2 = vbd_pcqt_to_qt[VBD_PCQT2];

      if constexpr (has_collision_force<qt1, qt2>()) {
        pcnt_s.template add_r2_external<qt1, qt2>(
          m_avbd_pcnt_set,
          dlm::ckey<tri_linearize(VBD_PCQT1, VBD_PCQT2, vbd_pcqt_num)>()
        );
      }
    });
  }

  template<unsigned QT1, unsigned QT2>
  inline void on_pcnt_add(unsigned reg, unsigned pcnt, unsigned q1, unsigned q2) {
    static constexpr bool qt1_gs = uses_gs_for_avbd_pcnt<qt_t<QT1, TW>>::value;
    static constexpr bool qt2_gs = uses_gs_for_avbd_pcnt<qt_t<QT2, TW>>::value;

    if constexpr (qt1_gs && qt2_gs) {
      auto &color_s = get<color_system>(*m_state);
      color_s.add_edge(reg, {QT1, q1}, {QT2, q2});
    }
  };

  template<unsigned QT1, unsigned QT2>
  inline void on_pcnt_null(unsigned reg, unsigned pcnt, unsigned q1, unsigned q2) {
    static constexpr bool qt1_gs = uses_gs_for_avbd_pcnt<qt_t<QT1, TW>>::value;
    static constexpr bool qt2_gs = uses_gs_for_avbd_pcnt<qt_t<QT2, TW>>::value;

    if constexpr (qt1_gs && qt2_gs) {
      auto &color_s = get<color_system>(*m_state);
      if (color_s.null_edge(reg, {QT1, q1}, {QT2, q2}) == 0) {
        color_s.clear_null_edges();
      }
    }
  };

  template<unsigned VBD_QT1, unsigned VBD_QT2>
  struct avbd_pcnt_tri_gen {
    static constexpr unsigned qt1 = vbd_pcqt_to_qt[VBD_QT1];
    static constexpr unsigned qt2 = vbd_pcqt_to_qt[VBD_QT2];
    static constexpr bool qt1_gs = uses_gs_for_avbd_pcnt<qt_t<qt1, TW>>::value;
    static constexpr bool qt2_gs = uses_gs_for_avbd_pcnt<qt_t<qt2, TW>>::value;

    using pcnt_data_t = avbd_pcnt_combo_t<cd_t<qt1, TW>, cd_t<qt2, TW>>;

    using type = dlm::ext<dlm::perm<
      dlm::comp_ds<dlm::vec_ds<pcnt_data_t>>,
      dlm::comp_ds<dlm::vec_ds<decltype([] {
        if constexpr (qt1_gs) {
          return avbd_q_to_cnt_t<pcnt_data_t>{};
        } else {
          return dlm::size_ds{};
        }
      }())>>,
      dlm::comp_ds<dlm::vec_ds<decltype([] {
        if constexpr (qt2_gs) {
          return avbd_q_to_cnt_t<pcnt_data_t>{};
        } else {
          return dlm::size_ds{};
        }
      }())>>
    >>;
  };

  // [reg]<vbd_qt1, vbd_qt2>[pcnt]
  struct avbd_pcnt_set: // [reg]
  dlm::ext<dlm::perm_tri_tuple<vbd_pcqt_num, avbd_pcnt_tri_gen>> {
    DEFINE_TRI_DS_FIELD((0), avbd_pcnt_data, VBD_QT, vbd_pcqt_num)
    DEFINE_TRI_DS_FIELD((1), q1_to_pcnt, VBD_QT, vbd_pcqt_num)
    DEFINE_TRI_DS_FIELD((2), q2_to_pcnt, VBD_QT, vbd_pcqt_num)
  } m_avbd_pcnt_set;

  god::state<TW> *m_state;
};

struct avbd_pcnt_fg {
  template<typename TW>
  using template_t = avbd_pcnt_fg_tt<TW>;
};
} // namespace cp6
