#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/avbd_system.h"
#include "collision/phase_link_system.h"
#include "cross_state.h"
#include "index_maps/comp_key_store.h"
#include "index_maps/dmap.h"
#include "index_maps/key_vec.h"
#include "index_maps/pkey_wrap.h"
#include "perm_tri_tuple.h"
#include "perm_tuple.h"
#include "region_state.h"
#include "tuple_utils.h"
#include <index_maps/split_key_vec.h>
#include <perm_arr.h>

namespace cp6 {
template<typename DST_QD, typename OTHER_QD, typename PF_STATE>
void add_pf_derivs_a(
  DST_QD &dst_qd,
  const OTHER_QD &other_qd,
  unsigned pf,
  const unsigned a,
  const unsigned b,
  const PF_STATE &pf_state
);

template<typename DST_QD, typename OTHER_QD, typename PF_STATE>
void add_pf_derivs_b(
  DST_QD &dst_qd,
  const OTHER_QD &other_qd,
  unsigned pf,
  const unsigned a,
  const unsigned b,
  const PF_STATE &pf_state
);

template<typename QD_A, typename QD_B, typename PF_STATE>
void update_pf_avbd_vars(
  PF_STATE &pf_state,
  unsigned q_a,
  unsigned q_b,
  unsigned pf,
  const QD_A &qt1,
  const QD_B &qt2
);

template<typename QD_A, typename QD_B, typename PF_STATE>
void init_pf_avbd_vars(
  PF_STATE &pf_state,
  unsigned q_a,
  unsigned q_b,
  unsigned pf,
  const QD_A &qt1,
  const QD_B &qt2
);

template<template<typename> class TRAIT, typename QD1, typename QD2>
struct pair_force_state;

template<template<typename> class TRAIT, typename QD1, typename QD2>
using pair_force_state_t = typename pair_force_state<TRAIT, QD1, QD2>::type;

// provides the common state and functions needed for forces/constraints between two coordinates.
template<template<typename> class TRAIT, typename... Ts>
struct pair_force_fg_tt: cp6::flag_phase_linker {
  static constexpr unsigned pfqt_num = get_matching_num<TRAIT, qt_ts<Ts...>>();
  static constexpr auto qt_to_pfqt = generate_tuple_indirection<
    qt_ts<Ts...>,
    god::get_matching_ts<TRAIT, qt_ts<Ts...>>,
    std::is_same
  >();
  static constexpr auto pfqt_to_qt = generate_tuple_indirection<
    god::get_matching_ts<TRAIT, qt_ts<Ts...>>,
    qt_ts<Ts...>,
    std::is_same
  >();
  static constexpr auto vbd_qt_to_qt = generate_tuple_indirection<
    god::get_matching_ts<is_vbd_qt, qt_ts<Ts...>>,
    qt_ts<Ts...>,
    std::is_same
  >();

  static constexpr unsigned vbd_qt_num = get_matching_num<is_vbd_qt, qt_ts<Ts...>>();

  template<unsigned QT1, unsigned QT2>
  const unsigned *pf_to_q1(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    return (const unsigned *)get<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_a_n>(m_pf_data)[reg]
      .template get<0>()
      .data();
  }

  template<unsigned QT1, unsigned QT2>
  const unsigned *pf_to_q2(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    return (const unsigned *)get<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_b_n>(m_pf_data)[reg]
      .template get<0>()
      .data();
  }

  // returns q's [q_pcnt_a]
  template<unsigned QT1, unsigned QT2>
  const std::vector<dlm::ikey<1>> &q1_to_pf(unsigned reg, unsigned q) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    return get<pfqt1, pfqt2 - pfqt1, 0>(m_a_q_map)[reg][q].get_base();
  }

  // returns q's [q_pf_b]
  template<unsigned QT1, unsigned QT2>
  const std::vector<dlm::ikey<1>> &q2_to_pf(unsigned reg, unsigned q) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    return get<pfqt2, pfqt1, 0>(m_b_q_map)[reg][q].get_base();
  }

  template<unsigned QT1, unsigned QT2>
  const auto &get_pf_state(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    static_assert(pfqt1 != -1 && pfqt2 != -1);
    return get<tri_linearize(pfqt1, pfqt2, pfqt_num), pf_state_n>(m_pf_data)[reg];
  }

  template<unsigned QT1, unsigned QT2>
  auto &get_pf_state(unsigned reg) {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];
    static_assert(pfqt1 != -1 && pfqt2 != -1);
    return get<tri_linearize(pfqt1, pfqt2, pfqt_num), pf_state_n>(m_pf_data)[reg];
  }

protected:
  friend class god::cross_state<Ts...>;
  friend class avbd_system_tt<Ts...>;
  friend class phase_link_system_tt<Ts...>;

  template<unsigned QT1, unsigned QT2, typename T>
  void add_phase_links(unsigned reg, T &phase_links) {
    // assert(false);
    for (unsigned reg = 0; reg < size(m_pf_data); reg++) {
    }
  }

  // return pf
  template<unsigned QT1, unsigned QT2>
  unsigned add_pf(unsigned reg, unsigned q_a, unsigned q_b) {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pfqt1 = qt_to_pfqt[QT1];
    static constexpr unsigned pfqt2 = qt_to_pfqt[QT2];

    unsigned pf = size<tri_linearize(pfqt1, pfqt2, pfqt_num)>(m_pf_data, {reg});

    grow<tri_linearize(pfqt1, pfqt2, pfqt_num)>(m_pf_data, 1, {reg});

    get<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_a_n>(m_pf_data).sublink(
      dlm::ikey{reg, pf},
      dlm::ikey{reg, q_a}
    );

    get<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_b_n>(m_pf_data).sublink(
      dlm::ikey{reg, pf},
      dlm::ikey{reg, q_b}
    );
    return pf;
  }

  void start_step(unsigned reg, float h) {
    tri_apply<pfqt_num>([&]<size_t PFQT1, size_t PFQT2> {
      static constexpr unsigned qt1 = pfqt_to_qt[PFQT1];
      static constexpr unsigned qt2 = pfqt_to_qt[PFQT2];
      auto &qt_state_a = god::get<is_region_state>(*m_state).template get_qt<qt1>(reg);
      auto &qt_state_b = god::get<is_region_state>(*m_state).template get_qt<qt2>(reg);

      for (unsigned pf = 0; pf < get_pf_state<qt1, qt2>(reg).size(); pf++) {
        init_pf_avbd_vars(
          get_pf_state<qt1, qt2>(reg),
          pf_to_q1<qt1, qt2>(reg)[pf],
          pf_to_q2<qt1, qt2>(reg)[pf],
          pf,
          qt_state_a.qd(),
          qt_state_b.qd()
        );
      }
    });
  };

  template<unsigned VBD_QT>
  void add_derivs(unsigned reg, unsigned q) {
    static constexpr unsigned pfqt = qt_to_pfqt[vbd_qt_to_qt[VBD_QT]];
    if constexpr (pfqt != -1) {

      for_each_i<pfqt_num - pfqt>([&]<size_t I> {
        static constexpr unsigned qt1 = pfqt_to_qt[pfqt];
        static constexpr unsigned qt2 = pfqt_to_qt[pfqt + I];

        const std::vector<dlm::ikey<1>> &q_to_pf = q1_to_pf<qt1, qt2>(reg, q);

        for (unsigned q_pf = 0; q_pf < q_to_pf.size(); q_pf++) {
          unsigned pf = q_to_pf[q_pf];
          add_pf_derivs_a(
            god::get<is_region_state>(*m_state).template get_qt<qt1>(reg).qd(),
            god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd(),
            pf_to_q1<qt1, qt2>(reg)[pf],
            pf_to_q2<qt1, qt2>(reg)[pf],
            pf,
            get_pf_state<qt1, qt2>(reg)
          );
        }
      });

      for_each_i<pfqt + 1>([&]<size_t OTHER_PFQT> {
        static constexpr unsigned qt1 = pfqt_to_qt[OTHER_PFQT];
        static constexpr unsigned qt2 = pfqt_to_qt[pfqt];

        const std::vector<dlm::ikey<1>> &q_to_pf = q2_to_pf<qt1, qt2>(reg, q);

        for (unsigned q_pf = 0; q_pf < q_to_pf.size(); q_pf++) {
          unsigned pf = q_to_pf[q_pf];
          add_pf_derivs_b(
            god::get<is_region_state>(*m_state).template get_qt<qt1>(reg).qd(),
            god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd(),
            pf_to_q2<qt1, qt2>(reg)[pf],
            pf_to_q1<qt1, qt2>(reg)[pf],
            pf,
            get_pf_state<qt1, qt2>(reg)
          );
        }
      });
    }
  }

  template<unsigned VBD_QT>
  void add_derivs(unsigned reg) {
    static constexpr unsigned pfqt = qt_to_pfqt[vbd_qt_to_qt[VBD_QT]];
    if constexpr (pfqt != -1) {
      auto &qt_state = god::get<is_region_state>(*m_state).template get_qt<VBD_QT>(reg);
      for_each_i<pfqt_num - pfqt>([&]<size_t I> {
        static constexpr unsigned qt1 = pfqt_to_qt[pfqt];
        static constexpr unsigned qt2 = pfqt_to_qt[pfqt + I];
        auto &pf_state = get_pf_state<qt1, qt2>(reg);
        for (unsigned pf = 0; pf < pf_state.size(); pf++) {
          add_pf_derivs_a(
            qt_state.qd(),
            god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd(),
            pf_to_q1<qt1, qt2>(reg)[pf],
            pf_to_q2<qt1, qt2>(reg)[pf],
            pf,
            pf_state
          );
        }
      });
      for_each_i<pfqt + 1>([&]<size_t OTHER_PFQT> {
        static constexpr unsigned qt1 = pfqt_to_qt[OTHER_PFQT];
        static constexpr unsigned qt2 = pfqt_to_qt[pfqt];
        auto &pf_state = get_pf_state<qt1, qt2>(reg);
        for (unsigned pf = 0; pf < pf_state.size(); pf++) {
          add_pf_derivs_b(
            qt_state.qd(),
            god::get<is_region_state>(*m_state).template get_qt<qt2>(reg).qd(),
            pf_to_q2<qt1, qt2>(reg)[pf],
            pf_to_q1<qt1, qt2>(reg)[pf],
            pf,
            pf_state
          );
        }
      });
    }
  }

  void after_step(unsigned reg, float h) {
    tri_apply<pfqt_num>([&]<size_t PFQT1, size_t PFQT2> {
      static constexpr unsigned qt1 = pfqt_to_qt[PFQT1];
      static constexpr unsigned qt2 = pfqt_to_qt[PFQT2];
      auto &qt_state_a = god::get<is_region_state>(*m_state).template get_qt<qt1>(reg);
      auto &qt_state_b = god::get<is_region_state>(*m_state).template get_qt<qt2>(reg);

      for (unsigned pf = 0; pf < get_pf_state<qt1, qt2>(reg).size(); pf++) {
        update_pf_avbd_vars(
          get_pf_state<qt1, qt2>(reg),
          pf_to_q1<qt1, qt2>(reg)[pf],
          pf_to_q2<qt1, qt2>(reg)[pf],
          pf,
          qt_state_a.qd(),
          qt_state_b.qd()
        );
      }
    });
  }

  void end_step(unsigned reg, float h) {};

  god::cross_state<Ts...> *m_state;

  void init(god::cross_state<Ts...> &state) {
    m_state = &state;

    tri_apply<pfqt_num>([&]<size_t pfqt1, size_t pfqt2> {
      dlm::init_maps(
        m_pf_data,
        m_a_q_map,
        dlm::ckey<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_a_n>{},
        dlm::ckey<pfqt1, pfqt2 - pfqt1, 0>{}
      );

      dlm::init_maps(
        m_pf_data,
        m_b_q_map,
        dlm::ckey<tri_linearize(pfqt1, pfqt2, pfqt_num), to_q_pf_b_n>{},
        dlm::ckey<pfqt2, pfqt1, 0>{}
      );
    });

    god::get<is_region_state>(state).add_r1_external(m_pf_data);
    god::get<is_region_state>(state).add_r1_external(m_a_q_map);
    god::get<is_region_state>(state).add_r1_external(m_b_q_map);
    for_each_i<pfqt_num>([&]<size_t pfqt> {
      god::get<is_region_state>(state).template add_r2_external<pfqt_to_qt[pfqt]>(
        m_a_q_map,
        dlm::ckey<pfqt>{}
      );
      god::get<is_region_state>(state).template add_r2_external<pfqt_to_qt[pfqt]>(
        m_b_q_map,
        dlm::ckey<pfqt>{}
      );
    });
  }

  template<index_t pfqt1, index_t pfqt2>
  struct x_tri_gen {
    using type = dlm::srq<dlm::perm< // <pfqt, pfqt>[x]
      dlm::dmap< // <0> to_q_pf_a
        dlm::pkey_wrap<dlm::comp_key_store<dlm::vec_ds<dlm::split_key_vec<1, 1>>>, 0, 0>
      >,
      dlm::dmap< // <1> to_q_pf_b
        dlm::pkey_wrap<dlm::comp_key_store<dlm::vec_ds<dlm::split_key_vec<1, 1>>>, 0, 0>
      >,
      dlm::vec_bundle_ds<dlm::perm_map<2>>, // <2> r2_externals
      dlm::comp_ds<                         // <3> x_state
        dlm::vec_ds<pair_force_state_t<
          TRAIT,
          qd_t<pfqt_to_qt[pfqt1], Ts...>,
          qd_t<pfqt_to_qt[pfqt2], Ts...>
        >>
      >
    >>;
  };

  static constexpr unsigned to_q_pf_a_n = 0;
  static constexpr unsigned to_q_pf_b_n = 1;
  static constexpr unsigned r2_externals_n = 2;
  static constexpr unsigned pf_state_n = 3;

  template<index_t PFQT>
  struct a_q_map_tuple_gen {
    // [q]<a>[q_pf]
    using type = dlm::ext<dlm::perm_arr< // [q]
      pfqt_num - PFQT,
      dlm::srq<dlm::perm<
        dlm::dmap<dlm::pkey_wrap<// <a>[q_pf_a]
          dlm::key_vec<3, 1>,
          0,
          0
        >>
      >>
    >>;
  };

  template<index_t PFQT>
  struct b_q_map_tuple_gen {
    // [q]<b>[q_pf]
    using type = dlm::ext<dlm::perm_arr<// [q]
      PFQT + 1,
      dlm::srq<dlm::perm<
        // [reg]<pfqt>[q]<b>[q_pf] <=> [reg]<pfqt, pfqt>[x]
        dlm::dmap<dlm::pkey_wrap<// <b>[q_pf_b]
          dlm::key_vec<3, 1>,
          0,
          0
        >>
      >>
    >>;
  };

  // [reg]<pfqt, pfqt>[x]
  struct pf_data: dlm::ext<dlm::perm_tri_tuple<pfqt_num, x_tri_gen>> {
  } m_pf_data;

  // [reg]<pfqt>[q]<a>[q_pf]
  struct a_q_map:
  dlm::ext<dlm::perm_tuple< // [reg]
    pfqt_num,
    a_q_map_tuple_gen // <pfqt>
  >> {
  } m_a_q_map;

  // [reg]<pfqt>[q]<b>[q_pf]
  struct b_q_map:
  dlm::ext<dlm::perm_tuple< // [reg]
    pfqt_num,
    b_q_map_tuple_gen // <pfqt>
  >> {
  } m_b_q_map;
};

template<template<typename> class TRAIT>
struct pair_force_fg {
  template<typename... Ts>
  using template_t = pair_force_fg_tt<TRAIT, Ts...>;
};

} // namespace cp6
