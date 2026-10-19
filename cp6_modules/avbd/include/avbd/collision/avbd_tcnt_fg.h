#pragma once
#include "avbd/avbd_flags.h"
#include "avbd_cnt_decl.h"
#include "god_state.h"
#include "operations.h"
#include "perm_tri_tuple.h"
#include "region_state.h"
#include "tri_utils.h"
#include "tuple_utils.h"
#include <avbd/avbd_system.h>
#include <collision/narrowphase_system.h>

namespace cp6 {
template<typename TW>
struct avbd_tnct_fg_tt {
  static constexpr bool vbd_jacobi_fg = true;

  static constexpr auto vbd_qt_to_qt = generate_tuple_indirection<
    get_matching_ts<is_vbd_qt, qt_ts<TW>>,
    qt_ts<TW>,
    std::is_same
  >();

  static constexpr auto qt_to_vbd_qt = generate_tuple_indirection<
    qt_ts<TW>,
    get_matching_ts<is_vbd_qt, qt_ts<TW>>,
    std::is_same
  >();
  static constexpr unsigned vbd_qt_num = get_matching_num<is_vbd_qt, qt_ts<TW>>();

  template<unsigned QT1, unsigned QT2>
  auto &get_avbd_tcnt_data(unsigned reg) {
    static constexpr unsigned vbd_qt1 = qt_to_vbd_qt[QT1];
    static constexpr unsigned vbd_qt2 = qt_to_vbd_qt[QT2];

    static_assert((uses_tcnt<QT1>() || uses_tcnt<QT2>()) && has_collision_force<QT1, QT2>());
    static_assert(has_collision_force<QT1, QT2>());
    if constexpr (uses_tcnt<QT1>() || uses_tcnt<QT2>()) {
      return m_avbd_tcnt_set.template avbd_tcnt_data<vbd_qt1, vbd_qt2>()[reg];
    } else {
    }
  }

private:
  friend struct god::state<TW>;
  friend class avbd_system_tt<TW>;

  template<unsigned QT>
  static constexpr bool uses_tcnt() {
    return uses_avbd_tcnt_force<qt_t<QT, TW>>::value;
  };

  template<unsigned QT1, unsigned QT2>
  static constexpr bool has_collision_force() {
    return has_collision_force_v<qt_t<QT1, TW>, qt_t<QT2, TW>>;
  }

  void start_step(unsigned reg, float h) {
    auto &reg_s = get<is_region_state>(*m_state);
    auto &np_s = get<narrowphase_system>(*m_state);

    // updating avbd_tcnt_data with new tcnt_data
    tri_apply<vbd_qt_num>([&]<size_t VBD_QT1, size_t VBD_QT2> {
      static constexpr unsigned qt1 = vbd_qt_to_qt[VBD_QT1];
      static constexpr unsigned qt2 = vbd_qt_to_qt[VBD_QT2];

      if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1, qt2>()) {
        dlm::clear<tri_linearize(VBD_QT1, VBD_QT2, vbd_qt_num)>(m_avbd_tcnt_set, reg);

        auto &qd1 = reg_s.template get_qt<qt1>(reg).qd();
        auto &qd2 = reg_s.template get_qt<qt2>(reg).qd();
        auto &avbd_tcnt_data = get_avbd_tcnt_data<qt1, qt2>(reg);
        const auto &tcnt_data = np_s.template get_tcnt_data<qt1, qt2>(reg);

        dlm::grow<tri_linearize(VBD_QT1, VBD_QT2, vbd_qt_num)>(
          m_avbd_tcnt_set,
          tcnt_data.size(),
          reg
        );

        init_avbd_state(qd1, qd2, tcnt_data, avbd_tcnt_data);
      }
    });
  };

  // template<unsigned VBD_QT>
  void add_derivs(unsigned reg) {
    // if constexpr (!uses_gs_for_tcnt<qt_t<vbd_qt_to_qt[VBD_QT], TW>>::value) {
    //   auto &np_s = get<narrowphase_system>(*m_state);
    //   auto &reg_s = get<is_region_state>(*m_state);
    //
    //   for_each_i<vbd_qt_num - VBD_QT>([&]<size_t I> {
    //     static constexpr unsigned vbd_qt1 = VBD_QT;
    //     static constexpr unsigned vbd_qt2 = VBD_QT + I;
    //     static constexpr unsigned qt1 = vbd_qt_to_qt[vbd_qt1];
    //     static constexpr unsigned qt2 = vbd_qt_to_qt[vbd_qt2];
    //
    //     if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1, qt2>())
    //     {
    //       auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
    //       auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
    //       auto &avbd_tcnt_data = m_avbd_tcnt_set.template avbd_tcnt_data<vbd_qt1,
    //       vbd_qt2>()[reg];
    //
    //       add_derivs1(qd1_s, qd2_s, avbd_tcnt_data);
    //     }
    //   });
    //
    //   for_each_i<VBD_QT + 1>([&]<size_t OTHER_VBD_QT> {
    //     static constexpr unsigned vbd_qt1 = OTHER_VBD_QT;
    //     static constexpr unsigned vbd_qt2 = VBD_QT;
    //     static constexpr unsigned qt1 = vbd_qt_to_qt[vbd_qt1];
    //     static constexpr unsigned qt2 = vbd_qt_to_qt[vbd_qt2];
    //
    //     if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1, qt2>())
    //     {
    //       auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
    //       auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
    //       auto &avbd_tcnt_data = m_avbd_tcnt_set.template avbd_tcnt_data<vbd_qt1,
    //       vbd_qt2>()[reg];
    //
    //       add_derivs2(qd1_s, qd2_s, avbd_tcnt_data);
    //     }
    //   });
    // }
    // static_assert(false, "not implemented");
  }

  // template<unsigned VBD_QT, typename COLOR_T>
  // void add_derivs_gs(unsigned reg, const COLOR_T &color) {
  //   if constexpr (!uses_gs_for_tcnt<qt_t<vbd_qt_to_qt[VBD_QT], TW>>::value) {
  //     auto &np_s = get<narrowphase_system>(*m_state);
  //     auto &reg_s = get<is_region_state>(*m_state);
  //
  //     for_each_i<vbd_qt_num - VBD_QT>([&]<size_t I> {
  //       static constexpr unsigned vbd_qt1 = VBD_QT;
  //       static constexpr unsigned vbd_qt2 = VBD_QT + I;
  //       static constexpr unsigned qt1 = vbd_qt_to_qt[vbd_qt1];
  //       static constexpr unsigned qt2 = vbd_qt_to_qt[vbd_qt2];
  //
  //       if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1,
  //       qt2>()) {
  //         auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
  //         auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
  //         auto &avbd_tcnt_data = m_avbd_tcnt_set.template avbd_tcnt_data<vbd_qt1,
  //         vbd_qt2>()[reg];
  //
  //         add_derivs1(qd1_s, qd2_s, avbd_tcnt_data, color);
  //       }
  //     });
  //
  //     for_each_i<VBD_QT + 1>([&]<size_t OTHER_VBD_QT> {
  //       static constexpr unsigned vbd_qt1 = OTHER_VBD_QT;
  //       static constexpr unsigned vbd_qt2 = VBD_QT;
  //       static constexpr unsigned qt1 = vbd_qt_to_qt[vbd_qt1];
  //       static constexpr unsigned qt2 = vbd_qt_to_qt[vbd_qt2];
  //
  //       if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1,
  //       qt2>()) {
  //         auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
  //         auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();
  //         auto &avbd_tcnt_data = m_avbd_tcnt_set.template avbd_tcnt_data<vbd_qt1,
  //         vbd_qt2>()[reg];
  //
  //         add_derivs2(qd1_s, qd2_s, avbd_tcnt_data, color);
  //       }
  //     });
  //   }
  // }

  void after_step(unsigned reg, float h) {
    auto &np_system = get<narrowphase_system>(*m_state);
    auto &reg_s = get<is_region_state>(*m_state);
    tri_apply<vbd_qt_num>([&]<size_t VBD_QT1, size_t VBD_QT2> {
      static constexpr unsigned qt1 = vbd_qt_to_qt[VBD_QT1];
      static constexpr unsigned qt2 = vbd_qt_to_qt[VBD_QT2];

      if constexpr ((uses_tcnt<qt1>() || uses_tcnt<qt2>()) && has_collision_force<qt1, qt2>()) {
        auto &avbd_tcnt_data = get_avbd_tcnt_data<qt1, qt2>(reg);
        auto &qd1_s = reg_s.template get_qt<qt1>(reg).qd();
        auto &qd2_s = reg_s.template get_qt<qt2>(reg).qd();

        update_avbd_vars(qd1_s, qd2_s, avbd_tcnt_data);
      }
    });
  }

  void end_step(unsigned reg, float h) {}

  void init(god::state<TW> &state) {
    m_state = &state;

    auto &region_state = get<is_region_state>(state);
    auto &np_system = get<narrowphase_system>(state);

    region_state.add_r1_external(m_avbd_tcnt_set);
  }

  template<unsigned VBD_QT1, unsigned VBD_QT2>
  struct tcnt_tri_gen {
    using type = dlm::perm< // <vbd_qt_a, vbd_qt_b>[tcnt]
      std::conditional_t<
        uses_tcnt<VBD_QT1>() || uses_tcnt<VBD_QT2>(),
        dlm::comp_ds<dlm::vec_ds<
          avbd_tcnt_combo_t<cd_t<vbd_qt_to_qt[VBD_QT1], TW>, cd_t<vbd_qt_to_qt[VBD_QT2], TW>>
        >>,
        dlm::vec_ds<float, 2>
      >
    >;
  };

  // [reg]<vbd_qt_a, vbd_qt_b>[tcnt]
  struct avbd_tcnt_set: // [reg]
  dlm::ext<dlm::perm_tri_tuple<vbd_qt_num, tcnt_tri_gen>> {
    DEFINE_TRI_DS_FIELD((0), avbd_tcnt_data, VBD_QT, vbd_qt_num)
  } m_avbd_tcnt_set;

  god::state<TW> *m_state;
};

struct avbd_tcnt_fg {
  static constexpr bool vbd_jacobi_fg = true;

  template<typename TW>
  using template_t = avbd_tnct_fg_tt<TW>;
};
} // namespace cp6
