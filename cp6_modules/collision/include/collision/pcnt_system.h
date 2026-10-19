#pragma once
#include "ab_link.h"
#include "collision_flags.h"
#include "collision_fn_decl.h"
#include "comp_ds.h"
#include "god_state.h"
#include "index_maps/base_map.h"
#include "index_maps/comp_key_store.h"
#include "index_maps/dmap.h"
#include "index_maps/key_vec.h"
#include "index_maps/pkey_wrap.h"
#include "index_maps/split_key_vec.h"
#include "narrowphase_system.h"
#include "operations.h"
#include "perm_arr.h"
#include "perm_tri_tuple.h"
#include "perm_tuple.h"
#include "region_state.h"
#include "task_marker.h"
#include "tri_utils.h"
#include "tuple_utils.h"
#include "vec_ds.h"

#include "phase_link_system.h"
#include <iostream>

namespace cp6 {
// persistant contact system
template<typename TW>
class pcnt_system_tt: public flag_phase_linker {
public:
  static constexpr unsigned pcqt_num = get_matching_num<is_pcqt, qt_ts<TW>>();
  static constexpr auto pcqt_to_qt = generate_tuple_indirection<
    get_matching_ts<is_pcqt, qt_ts<TW>>,
    qt_ts<TW>
  >();
  static constexpr auto qt_to_pcqt = generate_tuple_indirection<
    qt_ts<TW>,
    get_matching_ts<is_pcqt, qt_ts<TW>>
  >();

  template<unsigned QT1, unsigned QT2>
  const auto &get_pcnt_data(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);
    return m_pcnt_set.template pcnt_data<pcqt1, pcqt2>()[reg];
  }

  template<unsigned QT1, unsigned QT2>
  auto &get_pcnt_data(unsigned reg) {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);
    return m_pcnt_set.template pcnt_data<pcqt1, pcqt2>()[reg];
  }

  // map: [reg]<pcqt, pcqt>[pcnt] <=> [reg]<pcqt>[q]<q1>[q_pcnt_a]
  // map: [reg]<pcqt, pcqt>[pcnt] <=> [reg]<pcqt>[q]<q2>[q_pcnt_a]

  template<unsigned QT1, unsigned QT2>
  const unsigned *pcnt_to_q1(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    return (const unsigned *)m_pcnt_set
      .template to_q1_pcnt1<pcqt1, pcqt2>()[reg]
      .template get<0>()
      .data();
  }

  template<unsigned QT1, unsigned QT2>
  const unsigned *pcnt_to_q2(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    return (const unsigned *)m_pcnt_set
      .template to_q2_pcnt2<pcqt1, pcqt2>()[reg]
      .template get<0>()
      .data();
  }

  template<unsigned QT1, unsigned QT2>
  const unsigned *pcnt_to_q1_pcnt1(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    return (const unsigned *)m_pcnt_set
      .template to_q1_pcnt1<pcqt1, pcqt2>()[reg]
      .template get<1>()
      .data();
  }

  template<unsigned QT1, unsigned QT2>
  const unsigned *pcnt_to_q2_pcnt2(unsigned reg) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    return (const unsigned *)m_pcnt_set
      .template to_q2_pcnt2<pcqt1, pcqt2>()[reg]
      .template get<1>()
      .data();
  }

  // returns q's [q_pcnt_a]
  template<unsigned QT1, unsigned QT2>
  const std::vector<dlm::ikey<1>> &q1_to_pcnt(unsigned reg, unsigned q) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);
    return get<pcqt1, pcqt2 - pcqt1, 0>(m_q1_map)[reg][q].get_base();
  }

  // returns q's [q_pcnt_b]
  template<unsigned QT1, unsigned QT2>
  const std::vector<dlm::ikey<1>> &q2_to_pcnt(unsigned reg, unsigned q) const {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);
    return get<pcqt2, pcqt1, 0>(m_q2_map)[reg][q].get_base();
  }

  template<unsigned QT1, unsigned QT2, typename IS, rank_t P>
  void add_r2_external(IS &is, dlm::ckey<P> perm) {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);
    m_pcnt_set.template r2_externals<pcqt1, pcqt2>().add_ds(
      dlm::perm_map<2>{is, perm}
    );
  }

  template<unsigned QT1, unsigned QT2>
  void null_pcnt(unsigned reg, unsigned pcnt) {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    dlm::null<tri_linearize(pcqt1, pcqt2, pcqt_num)>(m_pcnt_set, {reg, pcnt});
  }

  template<unsigned QT1, unsigned QT2>
  void clear_null_pcnt() {
    static_assert(QT1 <= QT2);
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    static_assert(pcqt1 != -1 && pcqt2 != -1);

    dlm::clear_null(m_pcnt_set);
  }

  // can be performed before tcnts are generated
  void update_pt1(unsigned reg) {
    const auto &qt_state = get<is_region_state>(*m_state);

    // update the edges within the pcnt state
    tri_apply_spin<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
      static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
      static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];
      static_assert(
        is_plqt<qt_t<qt1, TW>>::value || is_plqt<qt_t<qt2, TW>>::value,
        "all persistant contacts must be phase linkable"
      );

      return update_pcnt_edges(
        pcnt_to_q1<qt1, qt2>(reg),
        pcnt_to_q2<qt1, qt2>(reg),
        get_pcnt_data<qt1, qt2>(reg)
      );
    });

    // Perform local narrowphase on persistent contacts
    tri_apply<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
      static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
      static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];
      static constexpr unsigned t = tri_linearize(PCQT1, PCQT2, pcqt_num);

      if (
        narrowphase_update(
          qt_state.template get_qt<qt1>(reg).cd(),
          qt_state.template get_qt<qt2>(reg).cd(),
          get_pcnt_data<qt1, qt2>(reg)
        )
      ) {
        task1.mark_as_done(tri_linearize(PCQT1, PCQT2, pcqt_num));
      };
    });
  }

  // must be called after tcnts are generated.
  void update_pt2(unsigned reg) {
    const auto &qt_state = get<is_region_state>(*m_state);

    // wait for narrowphases to return
    for (unsigned reg = 0; reg < m_pcnt_set.size(); reg++) {
      tri_apply_spin<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
        static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
        static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];
        static constexpr unsigned t = tri_linearize(PCQT1, PCQT2, pcqt_num);

        if (!task1.done(t)) {
          return narrowphase_update(
            qt_state.template get_qt<qt1>(reg).cd(),
            qt_state.template get_qt<qt2>(reg).cd(),
            get_pcnt_data<qt1, qt2>(reg)
          );
        } else {
          return true;
        }
      });
    }
  }

  // No reg argument because of clear_null
  void update_pt3() {
    // cull non-colliding from pcnt set. This part must be serial due to the
    // index sets potentially being interdependent.
    for (unsigned reg = 0; reg < m_pcnt_set.size(); reg++) {
      tri_apply<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
        auto &pcnt_data = m_pcnt_set.template pcnt_data<PCQT1, PCQT2>()[reg];
        for (unsigned pcnt = 0; pcnt < pcnt_data.size(); pcnt++) {
          if (pcnt_data.get_q1(pcnt) == -1) {
            // notify states that care
            static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
            static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];
            tuple_for_each(
              [&](auto &state) {
                if constexpr (requires {
                                state
                                  .template on_pcnt_null<qt1, qt2>(0, 0, 0, 0);
                              }) {
                  unsigned q1 = pcnt_to_q1<qt1, qt2>(reg)[pcnt];
                  unsigned q2 = pcnt_to_q2<qt1, qt2>(reg)[pcnt];
                  state.template on_pcnt_null<qt1, qt2>(reg, pcnt, q1, q2);
                }
              },
              m_state->m_states
            );
            null<tri_linearize(PCQT1, PCQT2, pcqt_num)>(
              m_pcnt_set,
              {reg, pcnt}
            );
          }
        }
      });
    }
    clear_null(m_pcnt_set);
  }

  void update_pt4(unsigned reg) {
    const auto &np = get<narrowphase_system>(*m_state);
    // append the tcnt data to the end of the pcnt data
    tri_apply_spin<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
      static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
      static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];

      const auto &tcnt_data = np.template get_tcnt_data<qt1, qt2>(reg);
      auto &pcnt_data = get_pcnt_data<qt1, qt2>(reg);
      unsigned old_pcnt_num = pcnt_data.size();

      grow<tri_linearize(PCQT1, PCQT2, pcqt_num)>(
        m_pcnt_set,
        tcnt_data.size(),
        {reg}
      );
      return copy_tcnt_to_pcnt(tcnt_data, old_pcnt_num, pcnt_data);
    });

    // link new pcnts
    tri_apply<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
      static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
      static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];

      const auto &tcnt_data = np.template get_tcnt_data<qt1, qt2>(reg);
      const auto &pcnt_data = get_pcnt_data<qt1, qt2>(reg);
      unsigned old_pcnt_num = pcnt_data.size() - tcnt_data.size();

      for (unsigned tcnt = 0; tcnt < tcnt_data.size(); tcnt++) {
        unsigned pcnt = old_pcnt_num + tcnt;
        unsigned q1 = tcnt_data.get_q1(tcnt);
        unsigned q2 = tcnt_data.get_q2(tcnt);

        auto &reg_s = get<is_region_state>(*m_state);
        // [reg]<pcqt1, pcqt2>[pcnt] <=> [reg]<pcqt1>[q1]
        m_pcnt_set.template to_q1_pcnt1<PCQT1, PCQT2>().sublink(
          dlm::ikey{reg, pcnt},
          dlm::ikey{reg, q1}
        );

        // [reg]<pcqt1, pcqt2>[pcnt] <=> [reg]<pcqt2>[q2]
        m_pcnt_set.template to_q2_pcnt2<PCQT1, PCQT2>().sublink(
          dlm::ikey{reg, pcnt},
          dlm::ikey{reg, q2}
        );

        // notify states that care
        static constexpr unsigned qt1 = pcqt_to_qt[PCQT1];
        static constexpr unsigned qt2 = pcqt_to_qt[PCQT2];
        tuple_for_each(
          [&](auto &state) {
            if constexpr (requires {
                            state.template on_pcnt_add<qt1, qt2>(0, 0, 0, 0);
                          }) {
              state.template on_pcnt_add<qt1, qt2>(reg, pcnt, q1, q2);
            }
          },
          m_state->m_states
        );
      }
    });
    task1.reset();
  }

  void update() {
    // can be done in parallel
    for (unsigned reg = 0; reg < m_pcnt_set.size(); reg++) {
      update_pt1(reg);
      update_pt2(reg);
    }
    // cannot be parallel reg due to the clear null
    update_pt3();
    // can be done in parallel
    for (unsigned reg = 0; reg < m_pcnt_set.size(); reg++) { update_pt4(reg); }
  }

private:
  friend struct god::state<TW>;
  friend class cp6::phase_link_system_tt<TW>;

  task_marker<tri_num(pcqt_num)> task1;

  template<unsigned QT1, unsigned QT2, typename T>
  bool add_phase_links(unsigned reg, T &phase_links) {
    static constexpr unsigned pcqt1 = qt_to_pcqt[QT1];
    static constexpr unsigned pcqt2 = qt_to_pcqt[QT2];
    if constexpr (pcqt1 != -1 && pcqt2 != -1) {
      unsigned pcnt_num = m_pcnt_set.template to_q1_pcnt1<pcqt1, pcqt2>()[reg]
                            .size();
      return add_pcnt_phase_links(
        pcnt_num,
        pcnt_to_q1<QT1, QT2>(reg),
        pcnt_to_q2<QT1, QT2>(reg),
        phase_links
      );
    } else {
      return true;
    }
  }

  template<unsigned DST_QT, unsigned SRC_QT>
  void on_q_move(unsigned dst_q, unsigned src_q, unsigned reg) {
    static constexpr unsigned dst_pcqt = qt_to_pcqt[DST_QT];
    static constexpr unsigned src_pcqt = qt_to_pcqt[SRC_QT];

    static constexpr unsigned qt1 = DST_QT <= SRC_QT ? DST_QT : SRC_QT;
    static constexpr unsigned qt2 = DST_QT <= SRC_QT ? SRC_QT : DST_QT;

    if constexpr (dst_pcqt != -1 && src_pcqt != -1) {
      if constexpr (can_qts_collide<qt1, qt2, TW>()) {
        // Eh, you might as well just regenerate the pcnts.
        std::cout << "on_q_move works for pcnt" << std::endl;
      }
    }
  }

  void init(god::state<TW> &state) {
    m_state = &state;
    auto &reg_s = get<is_region_state>(state);

    // initializing maps
    tri_apply<pcqt_num>([&]<size_t PCQT1, size_t PCQT2> {
      dlm::init_maps(
        m_pcnt_set,
        m_q1_map,
        pcnt_set::template to_q1_pcnt1_n<PCQT1, PCQT2>(),
        q1_map::template to_pcnt_n<PCQT1, PCQT2>()
      );

      dlm::init_maps(
        m_pcnt_set,
        m_q2_map,
        pcnt_set::template to_q2_pcnt2_n<PCQT1, PCQT2>(),
        q2_map::template to_pcnt_n<PCQT1, PCQT2>()
      );
    });

    reg_s.add_r1_external(m_pcnt_set);
    reg_s.add_r1_external(m_q1_map);
    reg_s.add_r1_external(m_q2_map);
    for_each_i<pcqt_num>([&]<size_t PCQT> {
      reg_s.template add_r2_external<pcqt_to_qt[PCQT]>(
        m_q1_map,
        dlm::ckey<PCQT>{}
      );
      reg_s.template add_r2_external<pcqt_to_qt[PCQT]>(
        m_q2_map,
        dlm::ckey<PCQT>{}
      );
    });
  }

  god::state<TW> *m_state;

  ///////////////////////////////////////////////////////////////////////////////////////////////

  // pcnt = persistant contact
  template<index_t PCQT1, index_t PCQT2>
  struct pcnt_tri_gen {
    using type = dlm::srq<dlm::perm< // <pcqt, pcqt>[pcnt]
      dlm::comp_ds<             // <0> pcnt_data 
        dlm::vec_ds<pcnt_combo_t<cd_t<pcqt_to_qt[PCQT1], TW>, cd_t<pcqt_to_qt[PCQT2], TW>>>
      >,
      dlm::dmap< // <1> to_q_pcnt_a
        dlm::pkey_wrap<dlm::comp_key_store<dlm::vec_ds<dlm::split_key_vec<1, 1>>>, 0, 0>
      >,
      dlm::dmap< // <2> to_q_pcnt_b
        dlm::pkey_wrap<dlm::comp_key_store<dlm::vec_ds<dlm::split_key_vec<1, 1>>>, 0, 0>
      >,
      dlm::vec_bundle_ds<dlm::perm_map<2>> // <3> r2_externals
    >>;
  };

  // [reg]<pcqt, pcqt>[pcnt]
  struct pcnt_set: dlm::ext<dlm::perm_tri_tuple<pcqt_num, pcnt_tri_gen>> {
    DEFINE_TRI_DS_FIELD((0), pcnt_data, PCQT, pcqt_num);
    DEFINE_TRI_DS_FIELD((1), to_q1_pcnt1, PCQT, pcqt_num);
    DEFINE_TRI_DS_FIELD((2), to_q2_pcnt2, PCQT, pcqt_num);
    DEFINE_TRI_DS_FIELD((3), r2_externals, PCQT, pcqt_num);
  } m_pcnt_set;

  ///////////////////////////////////////////////////////////////////////////////////////////////

  template<index_t PCQT>
  struct q1_map_tuple_gen {
    // [q]<q1>[q_pcnt]
    using type = dlm::ext<dlm::perm_arr< //<qt>[q]
      pcqt_num - PCQT,
      dlm::srq<dlm::perm<
        dlm::dmap<dlm::pkey_wrap<// <q1>[q_pcnt_a]
          dlm::key_vec<3, 1>,
          0,
          0
        >>
      >>
    >>;
  };

  // [reg]<pcqt>[q]<q1>[q_pcnt]
  struct q1_map: dlm::ext<dlm::perm_tuple< // [reg]
    pcqt_num,
    q1_map_tuple_gen // <pcqt>
  >>{
    template<unsigned PCQT1, unsigned PCQT2>
    static constexpr auto to_pcnt_n() {
      return dlm::ckey<PCQT1, PCQT2 - PCQT1, 0>{};
    }

    template<unsigned PCQT1, unsigned PCQT2>
    auto &to_pcnt() {
      return get(*this, to_pcnt_n<PCQT1, PCQT2>());
    }
  } m_q1_map;

  ///////////////////////////////////////////////////////////////////////////////////////////////

  template<index_t PCQT>
  struct q2_map_tuple_gen {
    // [q]<q2>[q_pcnt]
    using type = dlm::ext<dlm::perm_arr< // <pcqt>[q]
      PCQT + 1,
      dlm::srq<dlm::perm<
        // [reg]<pcqt>[q]<q2>[q_pcnt] -> [reg]<pcqt, pcqt>[pcnt]
        dlm::dmap<dlm::pkey_wrap<// <q2>[q_pcnt_b]
          dlm::key_vec<3, 1>,
          0,
          0
        >>
      >>
    >>;
  };

  // [reg]<pcqt>[q]<q2>[q_pcnt]
  struct q2_map: dlm::ext<dlm::perm_tuple< // [reg]
    pcqt_num,
    q2_map_tuple_gen // <pcqt>
  >>{
    template<unsigned PCQT1, unsigned PCQT2>
    static constexpr auto to_pcnt_n() {
      return dlm::ckey<PCQT2, PCQT1, 0>{};
    }

    template<unsigned PCQT1, unsigned PCQT2>
    auto &to_pcnt() {
      return get(*this, to_pcnt_n<PCQT1, PCQT2>());
    }
  } m_q2_map;
};

struct pcnt_system: flag_phase_linker {
  template<typename TW>
  using template_t = pcnt_system_tt<TW>;
};

} // namespace cp6

/*
  THOUGHTS:
    I could modify [reg]<pcqt1, pcqt2>[ps1][ps2][psc_pcnt] to only store valid
  ps ps pairs, but that's just additional complexity for not much gain.

    We should probably change
      [reg]<pcqt1>[ps1][psq1][psq2]<pcqt2>[psq1_psc_pcnt]
    to
      [reg]<pcqt1>[ps1][psq1]<pcqt2>[psq1_psc_pcnt]
    to remove redundent ps ps combos


    Our current set won't work, and the flip median is hard to maintain and not
  optimal. The problem is a qt can be any ps, but we are representing this
  relationship as if qt1 can only have the smaller ps. New sets: [reg]<qt1,
  qt2>[ps1][ps2][psc_pcnt] <=> [reg]<qt1, qt2>[pcnt] [reg]<qt1,
  qt2>[ps1][ps2][psc_pcnt] <=> [reg]<qt1>[ps1][psq1][ps2]<qt1>[psq1_psc_pcnt]
      [reg]<qt1, qt2>[ps1][ps2][psc_pcnt] <=>
  [reg]<qt2>[ps2][psq1][ps1]<qt2>[psq2_psc_pcnt]


    It is required that all pcnt_data be a data set. So, each pcnt_data should
  support the null operation. This means all the update_narrowphase function
  needs to return is which indices are null.

    Projected adjacency is better then dynamically maintained adjacency if
  multiple operations are called. At least in terms of the amount of memory
  getting moved around and simplicity.

    pcqt is a subset of plqt. But it really shouldn't. If only one of the bodies
  in a contact pair get's the persistant contact treatment, then we end up
  breaking some laws of physics.

    Edges will fuck up when q's are swapped around.
      For this to not happen we need these maps
        [reg]<plqt1, plqt2>[pcnt] <=> [reg]<plqt1>[q]<plqt2>[q_pcnt]
        [reg]<plqt1, plqt2>[pcnt] <=> [reg]<plqt2>[q]<plqt1>[q_pcnt]

      Instead of being implicit, we can make the second plqt part of
      the inverse. So, each q will carry a single list of {plqt, pcnt}.
      This is probably better since each q will only collide with a few
      other qs
        [reg]<plqt1>[plqt2][pcnt][a/b] <=> [reg]<plqt1>[q][q_pcnt]
        [reg]<plqt2>[plqt1][pcnt][a/b] <=> [reg]<plqt2>[q][q_pcnt]

      It is better to split the set up into as and bs because the functions
      that act on a's are slightly different than those that act on b's.
        <q1>[reg]<plqt1>[plqt2][pcnt] <=> <q1>[reg]<plqt1>[q][q_pcnt]
        <q2>[reg]<plqt1>[plqt2][pcnt] <=> <q2>[reg]<plqt1>[q][q_pcnt]

        <q1>[reg]<plqt2>[plqt1][pcnt] <=> <q1>[reg]<plqt2>[q][q_pcnt]
        <q2>[reg]<plqt2>[plqt1][pcnt] <=> <q2>[reg]<plqt2>[q][q_pcnt]

      Actually, much of the code that uses contacts are specilized per <qt, qt>
  pair. So, not having it implicit would require some sort of dynamic dispatch.
  However even with it implicit, the inverse maps are not cache coherent like we
  want. To achieve this cache coherency we could do an inverse projection onto
  [reg]<plqt, plqt>[adj] whenever the  sets are changed. It would be better if
  we could make [q]<plqt2>[q_pcnt] in [reg]<plqt1>[q]<plqt2>[q_pcnt] coherent.

      Since our goal is to get a feature complete prototype, it is best we stick
  to the non-coherent non-projected implicit method, so this: <q1>[reg]<plqt1,
  plqt2>[pcnt] <=> <q1>[reg]<plqt1>[q]<plqt2>[q_pcnt] <q2>[reg]<plqt1,
  plqt2>[pcnt] <=> <q2>[reg]<plqt1>[q]<plqt2>[q_pcnt]

        <q1>[reg]<plqt1, plqt2>[pcnt] <=> <q1>[reg]<plqt2>[q]<plqt1>[q_pcnt]
        <q2>[reg]<plqt1, plqt2>[pcnt] <=> <q2>[reg]<plqt2>[q]<plqt1>[q_pcnt]
      Implicit doesn't mean permute.
        <q1>[reg]<plqt1>[plqt2][pcnt] <=> <q1>[reg]<plqt1>[q][plqt2][q_pcnt]
        <q2>[reg]<plqt1>[plqt2][pcnt] <=> <q2>[reg]<plqt1>[q][plqt2][q_pcnt]

        <q1>[reg]<plqt1>[plqt2][pcnt] <=> <q1>[reg]<plqt2>[q][plqt1][q_pcnt]
        <q2>[reg]<plqt1>[plqt2][pcnt] <=> <q2>[reg]<plqt2>[q][plqt1][q_pcnt]

        This way we can use the r2_coherent key store

    I didn't take into account that a set being a or b is implicitly tied with
  it's qt index. This means each qt has two sets: a set of qts that it is a in
  and a set of qts that it is b in.

      For example, say I have the qt set <qt0, qt1, qt2>, these are the ab sets
      qt0 a set: {qt0, qt1, qt2}
      qt0 b set: {qt0}
      qt1 a set: {qt1, qt2}
      qt1 b set: {qt0, qt1}
      qt2 a set: {qt2}
      qt2 b set: {qt0, qt1, qt2}

      It is these sets that the [reg]<plqt1, plqt2>[pcnt] must map to:
        [reg]<plqt1, plqt2>[pcnt] <=> [reg]<plqt1>[q]<a = [plqt2 - plqt1]>[pcnt]
        [reg]<plqt1, plqt2>[pcnt] <=> [reg]<plqt2>[q]<b = [plqt1]>[pcnt]

      to_q_a<qt1, qt2>(reg)
      to_q_b<qt1, qt2>(reg)

      to_pcnt_a<qt1, qt2>(reg, q_a)
      to_pcnt_b<qt1, qt2>(reg, q_b)




    OVERALL PROCESS:
      update broad-phase
      add_phase links
      use phase links to cull potential collisions from broad-phase
      perform global narrow-phase on potential collisions
      update local narrow-phase
        This step could be done in parrallel with broadphase
      remove any non-colliding persistent contacts.
      Add the new contacts to persistent contacts

    pulling in new contacts from narrowphase state implies there is a way to
  append to the end of col state. If we have two different routes for
  proccessing contacts, then we could pull in all the contacts from the last
  frame and add them here.

      OVERALL PROCESS 2:
        Add old contacts to persistent contacts
        update local narrow-phase
        remove any non-colliding persistent contacts.

        update broad-phase
        add_phase links
        use phase links to cull potential collisions from broad-phase
        perform global narrow-phase on potential collisions

      Doing it this way would provide a natural place to put an impulse like
      solver for collisions.
*/

/*
  // [reg][psc][psc_pcnt] -> [reg][ps1][psq1][psq1_psc_pcnt]
  // [reg][psc][psc_pcnt] -> [reg][ps2][psq2][psq2_psc_pcnt]
  // each rank of the inverse key is stored in a seperate vector. Each rank of
the both maps
  // are stored in the same vector. hehe
  struct psc_store_t:
  dlm::comp_ds<dlm::vec_ds<dlm::arr_ds<
    psc_num,
    dlm::dst<
      dlm::vec_ds<ab_link>, // [ps1/ps2]
      dlm::vec_ds<ab_link>, // [psq1/psq2]
      dlm::vec_ds<ab_link>  // [psq1_psc_pcnt/psq2_psc_pcnt]
    >
  >>> {
    static constexpr rank_t rank = 3;
    static constexpr index_t map_num = 2;
    static constexpr rank_t inv_ranks[] = {4, 4};

    template<index_t I>
    auto get_inverse(dlm::ikey<rank> key) const {
      const auto &root = this->operator[](key.cut_end());
      return dlm::ikey{
        key[0],
        get<I>(get<0>(root)[key.end()]),
        get<I>(get<1>(root)[key.end()]),
        get<I>(get<2>(root)[key.end()])
      };
    }

    template<index_t I>
    auto set_inverse(dlm::ikey<rank> key, dlm::ikey<4> value) {
      auto &root = this->operator[](key.cut_end());
      get<I>(get<0>(root)[key.end()]) = value[1];
      get<I>(get<1>(root)[key.end()]) = value[2];
      get<I>(get<2>(root)[key.end()]) = value[3];
    }

    template<index_t I>
    bool is_0_valid(dlm::ikey<rank> key) const {
      auto &root = this->operator[](key.cut_end());
      return get<I>(get<0>(root)[key.end()]) != -1;
    }
  };

  template<unsigned PCQT1, unsigned PCQT2>
  struct psc_tri_gen {
    using type = dlm::perm< // <qt1, qt2>[psc]
      dlm::srq<dlm::perm<   // [psc_cnt]
                            // <0> psc_pcnt_to_pscq
        // [reg]<pcqt1, pcqt2>[psc][psc_pcnt] -> [reg]<pcqt1, pcqt2>[pcnt]
        dlm::dmap<dlm::pkey_wrap<
          dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<psc_num, dlm::key_vec<1,
1>>>>, 0,
          0
        >>,
        // For when psq changes:
        // [reg]<pcqt1, pcqt2>[psc][psc_pcnt] ->
<pcqt1>[ps1][psq1][psq1_psc_pcnt]<q1>
        // [reg]<pcqt1, pcqt2>[psc][psc_pcnt] ->
<pcqt2>[ps2][psq2][psq2_psc_pcnt]<q2> dlm::multi_dmap<psc_store_t>
      >>,
      // median for when the ab_links need to be interpreted as ba_links
      dlm::comp_ds<dlm::vec_ds<dlm::arr_ds<psc_num, unsigned>>>
    >;
  };

  // [reg]<qt1, qt2>[psc][psc_pcnt]
  struct psc_data: dlm::ext<dlm::perm_tri_tuple<pcqt_num, psc_tri_gen>> {
    DEFINE_TRI_DS_FIELD((1), flip_median, PCQT, pcqt_num);
    DEFINE_TRI_DS_FIELD((0, 0), to_pcnt, PCQT, pcqt_num);
    DEFINE_TRI_DS_FIELD((0, 1), to_psq, PCQT, pcqt_num);
  } m_psc_set;

  template<unsigned PCQT>
  struct a_psc_map_tuple_gen {
    using type =
      dlm::ext<dlm::perm< // <pcqt>[ps]
        dlm::ext<dlm::perm< // [psq]
          dlm::srq<dlm::perm_arr< // [psq_psc_pcnt]<q1>
            pcqt_num - PCQT,
            // For when psq changes:
            //  [reg]<pcqt1>[ps1][psq][psq2_psc_pcnt]<q1> -> [reg]<pcqt1,
pcqt2>[psc][psc_pcnt]
            dlm::dmap<dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<ps_num,
dlm::vec_ds<dlm::key_vec<1, 3>>>>>>
          >>
        >>
      >>;
  };

  // [reg]<pcqt>[ps][psq][psq_psc_pcnt]<q1>
  struct a_psc_map: dlm::ext<dlm::perm_tuple<// [reg]
    pcqt_num,
    a_psc_map_tuple_gen
  >>{
    DEFINE_TUPLE_DS_FIELD((0, 0, 0), to_psc_pcnt, PCQT);
  } m_a_psc_map;

  template<unsigned PCQT>
  struct b_psc_map_tuple_gen {
    using type =
      dlm::ext<dlm::perm< // <pcqt>[ps]
        dlm::ext<dlm::perm< // [psq]
          dlm::srq<dlm::perm_arr< // [psq_psc_pcnt]<q2>
            PCQT + 1,
            // For when psq changes:
            //  [reg]<pcqt2>[ps2][psq2][psq2_psc_pcnt]<q2> -> [reg]<pcqt1,
pcqt2>[psc][psc_pcnt]
            dlm::dmap<dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<ps_num,
dlm::vec_ds<dlm::key_vec<1, 3>>>>>>
          >>
        >>
      >>;
  };

  // [reg]<pcqt>[ps][psq][psq_psc_pcnt]<q2>
  struct b_psc_map: dlm::ext<dlm::perm_tuple<// [reg]
    pcqt_num,
    b_psc_map_tuple_gen
  >>{
    DEFINE_TUPLE_DS_FIELD((0, 0, 0), to_psc_pcnt, PCQT)
  } m_b_psc_map;

*/
