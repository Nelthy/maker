#pragma once
#include "avbd/avbd_flags.h"
#include "collision/narrowphase_system.h"
#include "collision/pcnt_system.h"
#include "cross_state.h"
#include "external_perm.h"
#include "maths.h"
#include "operations.h"
#include "perm_tri_tuple.h"
#include "region_state.h"
#include "tri_utils.h"
#include "tuple_utils.h"

// needed for definition
#include "impulse_impl.h"
#include "impulse_states.h"

namespace cp6 {
template<typename IMP_DATA, typename CNT_DATA, typename Q_DATA_A, typename Q_DATA_B>
void init_impulse_data(
  IMP_DATA &imp_data,
  unsigned q_a,
  unsigned q_b,
  unsigned imp,
  unsigned cnt,
  const Q_DATA_A &q_a_data,
  const Q_DATA_B &q_b_data,
  const CNT_DATA &cnt_data
);

template<typename QD_A, typename QD_B, typename IMP_DATA>
void add_impulse(
  QD_A &qd_a,
  QD_B &qd_b,
  unsigned q_a,
  unsigned q_b,
  unsigned imp,
  IMP_DATA &imp_data
);

template<typename QD, typename IMP_DATA>
void apply_impulses(QD &dst_qd, unsigned q, unsigned imp, const IMP_DATA &imp_data);

template<typename QD1, typename QD2, typename CNT_DATA>
bool use_impulse(
  unsigned q_a,
  unsigned q_b,
  unsigned cnt,
  const QD1 &q_a_data,
  const QD2 &q_b_data,
  const CNT_DATA &cnt_data
);

template<typename T>
struct uses_impulses {
  static constexpr bool value = requires { requires(T::use_impulses); };
};

// pulls from the narrowphase system and calls the apply impulse function
template<typename... Ts>
struct impulse_system_tt {
  static constexpr unsigned qt_num = cp6::get_qt_num<Ts...>();
  static constexpr unsigned iqt_num = get_matching_num<uses_impulses, qt_ts<Ts...>>();

  static constexpr auto iqt_to_qt = generate_tuple_indirection<
    god::get_matching_ts<uses_impulses, qt_ts<Ts...>>,
    qt_ts<Ts...>,
    std::is_same
  >();

  static constexpr auto qt_to_iqt = generate_tuple_indirection<
    qt_ts<Ts...>,
    god::get_matching_ts<uses_impulses, qt_ts<Ts...>>,
    std::is_same
  >();

  template<unsigned QT1, unsigned QT2>
  auto &get_imp_data(unsigned reg) {
    static constexpr unsigned iqt1 = qt_to_iqt[QT1];
    static constexpr unsigned iqt2 = qt_to_iqt[QT2];
    static_assert(iqt1 != -1 && iqt2 != -1);
    return m_imp_set.template imp_data<iqt1, iqt2>()[reg];
  }

  template<unsigned QT1, unsigned QT2>
  const auto &get_imp_data(unsigned reg) const {
    static constexpr unsigned iqt1 = qt_to_iqt[QT1];
    static constexpr unsigned iqt2 = qt_to_iqt[QT2];
    static_assert(iqt1 != -1 && iqt2 != -1);
    return m_imp_set.template imp_data<iqt1, iqt2>()[reg];
  }

  void start_step(unsigned reg, float h) {
    // updating imp data
    tri_apply<iqt_num>([&]<size_t IQT1, size_t IQT2> {
      static constexpr unsigned qt1 = iqt_to_qt[IQT1];
      static constexpr unsigned qt2 = iqt_to_qt[IQT2];

      dlm::clear<tri_linearize(IQT1, IQT2, iqt_num)>(m_imp_set, {reg});

      if constexpr (uses_persistent_contacts<qt_t<qt1, Ts...>>::value &&
                    uses_persistent_contacts<qt_t<qt2, Ts...>>::value) {
        const auto &pcnt_s = god::get<pcnt_system>(*m_state);
        const auto &pcnt_data = pcnt_s.template get_pcnt_data<qt1, qt2>(reg);

        dlm::grow<tri_linearize(IQT1, IQT2, iqt_num)>(m_imp_set, pcnt_data.size(), {reg});
        auto &imp_data = m_imp_set.template tri_get<IQT1, IQT2, impulse_data_n>()[reg];
        auto &imp_to_pcnt = m_imp_set.template tri_get<IQT1, IQT2, to_cnt>()[reg];
        auto &q_a_data = god::get<is_region_state>(*m_state).template get_qt<qt1>(reg).qd();
        auto &q_b_data = god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd();

        for (unsigned imp = 0; imp < imp_data.size(); imp++) {
          unsigned pcnt = imp_to_pcnt[imp];
          unsigned q_a = pcnt_s.template pcnt_to_q_a<qt1, qt2>(reg)[pcnt];
          unsigned q_b = pcnt_s.template pcnt_to_q_b<qt1, qt2>(reg)[pcnt];
          init_impulse_data(imp_data, q_a, q_b, imp, pcnt, q_a_data, q_b_data, pcnt_data);
        }
      } else {
        const auto &cnt_set = god::get<narrowphase_system>(*m_state);
        const auto &cnt_data = cnt_set.template get_col_state<qt1, qt2>(reg);
        auto &imp_to_cnt = m_imp_set.template tri_get<IQT1, IQT2, to_cnt>()[reg];
        auto &imp_data = m_imp_set.template tri_get<IQT1, IQT2, impulse_data_n>()[reg];
        auto &q_a_data = god::get<is_region_state>(*m_state).template get_qt<qt1>(reg).qd();
        auto &q_b_data = god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd();
        for (unsigned imp = 0; imp < imp_data.size(); imp++) {
          unsigned cnt = imp_to_cnt[imp];
          unsigned q_a = cnt_set.template get_edges<qt1, qt2>(reg)[cnt].a;
          unsigned q_b = cnt_set.template get_edges<qt1, qt2>(reg)[cnt].b;
          init_impulse_data(imp_data, q_a, q_b, imp, cnt, q_a_data, q_b_data, cnt_data);
        }
      }
    });
  }

  void step(unsigned reg, float h) {
    tri_apply<iqt_num>([&]<size_t IQT_A, size_t IQT_B> {
      static constexpr unsigned qt_a = iqt_to_qt[IQT_A];
      static constexpr unsigned qt_b = iqt_to_qt[IQT_B];

      if constexpr (uses_persistent_contacts<qt_t<qt_a, Ts...>>::value &&
                    uses_persistent_contacts<qt_t<qt_b, Ts...>>::value) {
        const auto &pcnt_s = god::get<pcnt_system>(*m_state);
        const auto &pcnt_data = pcnt_s.template get_col_state<qt_a, qt_b>(reg);
        auto &imp_to_pcnt = m_imp_set.template tri_get<IQT_A, IQT_B, to_cnt>()[reg];
        auto &q_a_data = god::get<is_region_state>(*m_state).template get_qt<qt_a>(reg).qd();
        auto &q_b_data = god::get<is_region_state>(*m_state).template get_qt<qt_b>(reg).qd();

        for (unsigned imp = 0; imp < imp_to_pcnt.size(); imp++) {
          unsigned pcnt = imp_to_pcnt[imp];
          add_impulse(
            q_a_data,
            q_b_data,
            pcnt_s.template pcnt_to_q_a<qt_a, qt_b>(reg)[pcnt],
            pcnt_s.template pcnt_to_q_b<qt_a, qt_b>(reg)[pcnt],
            imp,
            m_imp_set.template tri_get<IQT_A, IQT_B, impulse_data_n>()[reg]
          );
        }
      } else {
        const auto &cnt_set = god::get<narrowphase_system>(*m_state);
        const auto &cnt_data = cnt_set.template get_col_state<qt_a, qt_b>(reg);
        auto &imp_to_cnt = m_imp_set.template tri_get<IQT_A, IQT_B, to_cnt>()[reg];
        auto &q_a_data = god::get<is_region_state>(*m_state).template get_qt<qt_a>(reg).qd();
        auto &q_b_data = god::get<is_region_state>(*m_state).template get_qt<qt_b>(reg).qd();

        for (unsigned imp = 0; imp < imp_to_cnt.size(); imp++) {
          unsigned cnt = imp_to_cnt[imp];
          add_impulse(
            q_a_data,
            q_b_data,
            cnt_set.template get_edges<qt_a, qt_b>(reg)[cnt].a,
            cnt_set.template get_edges<qt_a, qt_b>(reg)[cnt].b,
            imp,
            m_imp_set.template tri_get<IQT_A, IQT_B, impulse_data_n>()[reg]
          );
        }
      }
    });
  }

  void end_step(unsigned reg, float h) {
    // tri_apply<iqt_num>([&]<size_t IQT_A, size_t IQT_B> {
    //   static constexpr unsigned qt_a = iqt_to_qt[IQT_A];
    //   static constexpr unsigned qt_b = iqt_to_qt[IQT_B];
    //
    //   if constexpr (uses_persistent_contacts<qt_t<qt_a, Ts...>>::value &&
    //                 uses_persistent_contacts<qt_t<qt_b, Ts...>>::value) {
    //
    //     auto &pcnt_set = god::get<persistent_contact_system>(*m_state);
    //     auto &imp_to_pcnt = m_imp_set.template tri_get<IQT_A, IQT_B, to_cnt>()[reg];
    //
    //     for (unsigned imp = 0; imp < imp_to_pcnt.size(); imp++) {
    //       pcnt_set.template null_pcnt<qt_a, qt_b>(reg, imp_to_pcnt[imp]);
    //     }
    //     pcnt_set.template clear_null_pcnt<qt_a, qt_b>();
    //   }
    // });
  }

private:
  friend class god::cross_state<Ts...>;

  void init(god::cross_state<Ts...> &state) {
    m_state = &state;
    god::get<is_region_state>(state).add_r1_external(m_imp_set);
  }

  template<unsigned IQT1, unsigned IQT2>
  struct tri_gen {
    using type = dlm::perm<dlm::comp_ds<
      dlm::vec_ds<imp_combo_t<cd_t<iqt_to_qt[IQT1], Ts...>, cd_t<iqt_to_qt[IQT2], Ts...>>>
    >>;
  };

  // [reg]<iqt, iqt>[imp]
  struct impulse_set: dlm::ext<dlm::perm_tri_tuple<iqt_num, tri_gen>> {
    DEFINE_TRI_DS_FIELD((0), imp_data, IQT, iqt_num);
  } m_imp_set;

  god::cross_state<Ts...> *m_state;
};

struct impulse_system {
  template<typename... Ts>
  using template_t = impulse_system_tt<Ts...>;
};
} // namespace cp6

#define DEFINE_IMPULSE_STATE(qd_a, qd_b, impulse_state)                                            \
  struct impulse_state_combo<qd_a, qd_b> {                                                         \
    using type = impulse_state;                                                                    \
  }
