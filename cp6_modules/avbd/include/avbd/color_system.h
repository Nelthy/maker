#pragma once
#include "arr_ds.h"
#include "color_decl.h"
#include "comp_ds.h"
#include "ds_utils.h"
#include "god_state.h"
#include "ikey.h"
#include "index_maps/base_map.h"
#include "index_maps/comp_key_store.h"
#include "index_maps/dmap.h"
#include "index_maps/imap.h"
#include "index_maps/key_vec.h"
#include "index_maps/merge_key_wrap.h"
#include "index_maps/split_key_vec.h"
#include "operations.h"
#include "print_functions.h"
#include "region_state.h"
#include "relay/pointer_relay_ds.h"
#include "relay/receiver_modifier.h"
#include "shift_set_remove_queue.h"
#include "tuple_utils.h"
#include "vec_ds.h"

namespace cp6 {

// if false, the dst index is the index x would be be BEFORE.
template<typename T, typename VALUE_T>
static inline bool bin_search(
  const T &arr,
  const VALUE_T &x,
  unsigned &dst_index,
  unsigned start,
  unsigned end
) {
  int low = start;
  int high = end;
  while (low <= high) {
    int mid = low + (high - low) / 2;

    // Check if x is present at mid
    if (arr[mid] == x) {
      dst_index = mid;
      return true;
    };

    // If x greater, ignore left half
    if (arr[mid] < x) {
      low = mid + 1;
    } else {
      // If x is smaller, ignore right half
      high = mid - 1;
    }
  }

  // If we reach here, then element was not present
  dst_index = low;
  return false;
}

template<typename TW>
struct color_system_tt {
  static constexpr bool is_color_state = true;
  static constexpr unsigned cqt_num = get_matching_num<is_cqt, qt_ts<TW>>();
  static constexpr auto cqt_to_qt =
    generate_tuple_indirection<get_matching_ts<is_cqt, qt_ts<TW>>, qt_ts<TW>>();
  static constexpr auto qt_to_cqt =
    generate_tuple_indirection<qt_ts<TW>, get_matching_ts<is_cqt, qt_ts<TW>>>();

  unsigned removed_edge_update_threshold = 1;

  // returns the edge saturation
  unsigned add_edge(dlm::ikey<1> reg, dlm::ikey<2> qt1_q1, dlm::ikey<2> qt2_q2) {
    qt1_q1[0] = qt_to_cqt[qt1_q1[0]];
    qt2_q2[0] = qt_to_cqt[qt2_q2[0]];

    assert(qt1_q1.is_valid());
    assert(qt2_q2.is_valid());
    dlm::ikey<3> q1 = {reg, qt1_q1};
    dlm::ikey<3> q2 = {reg, qt2_q2};

    auto &q1qs = m_q_set.to_qq()[q1].template get<0>();
    auto &q2qs = m_q_set.to_qq()[q2].template get<0>();

    dlm::ikey<1> dst_q1q2;
    dlm::ikey<1> dst_q2q1;
    bool has_edge = false;
    if (bin_search(q1qs, qt2_q2, dst_q1q2[0], 0, q1qs.size() - 1)) has_edge = true;
    if (bin_search(q2qs, qt1_q1, dst_q2q1[0], 0, q2qs.size() - 1) != has_edge)
      assert(false && "invalid map");

    dlm::ikey<4> q1q2 = {q1, dst_q1q2};
    dlm::ikey<4> q2q1 = {q2, dst_q2q1};

    if (!has_edge) {
      dlm::shift_right(m_q_set, q1q2, 1, q_set::to_qq_n.cut_end());
      dlm::shift_right(m_q_set, q2q1, 1, q_set::to_qq_n.cut_end());
      m_q_set.to_qq().link(q1q2, q2q1);

      // adding to update set
      if (!m_q_set.to_update_q().is_0_valid(q1)) {
        m_q_set.to_update_q().sublink(q1, q1.cut_end());
      }
      if (!m_q_set.to_update_q().is_0_valid(q2)) {
        m_q_set.to_update_q().sublink(q2, q2.cut_end());
      }

      unsigned c1 = m_q_set.to_cq()[q1[0]][q1[1]].template get<0>()[q1[2]];
      unsigned c2 = m_q_set.to_cq()[q2[0]][q2[1]].template get<0>()[q2[2]];
      if (c1 != -1) {
        m_color_set.update_color_data()[q1[0]][c1][q1[1]] = true;
      }
      if (c2 != -1) {
        m_color_set.update_color_data()[q2[0]][c2][q2[1]] = true;
      }
      m_q_set.edge_saturation()[q1q2] = 0;
      m_q_set.edge_saturation()[q2q1] = 0;
    }
    m_q_set.edge_saturation()[q1q2]++;
    m_q_set.edge_saturation()[q2q1]++;
    // std::cout << "add edge: " << q1 << "<->" << q2 << " sats: " <<
    // m_q_set.edge_saturation()[q1q2]
    //           << " " << m_q_set.edge_saturation()[q2q1] << std::endl;

    return m_q_set.edge_saturation()[q1q2];
  }

  // returns the edge saturation. Only nulls once saturation is 0.
  unsigned null_edge(dlm::ikey<1> reg, dlm::ikey<2> qt1_q1, dlm::ikey<2> qt2_q2) {
    qt1_q1[0] = qt_to_cqt[qt1_q1[0]];
    qt2_q2[0] = qt_to_cqt[qt2_q2[0]];
    assert(qt1_q1.is_valid());
    assert(qt2_q2.is_valid());
    dlm::ikey<3> q1 = {reg, qt1_q1};
    dlm::ikey<3> q2 = {reg, qt2_q2};

    auto &q1qs = m_q_set.to_qq()[q1].template get<0>();
    auto &q2qs = m_q_set.to_qq()[q2].template get<0>();

    dlm::ikey<1> dst_q1q2;
    dlm::ikey<1> dst_q2q1;
    bool has_edge = false;
    if (bin_search(q1qs, qt2_q2, dst_q1q2[0], 0, q1qs.size() - 1)) has_edge = true;
    if (bin_search(q2qs, qt1_q1, dst_q2q1[0], 0, q2qs.size() - 1) != has_edge)
      assert(false && "invalid map");

    dlm::ikey<4> q1q2 = {q1, dst_q1q2};
    dlm::ikey<4> q2q1 = {q2, dst_q2q1};

    assert(has_edge);

    unsigned &q1q2_edge_sat = m_q_set.edge_saturation()[q1q2];
    unsigned &q2q1_edge_sat = m_q_set.edge_saturation()[q2q1];

    // std::cout << "null edge: " << q1 << "<->" << q2 << " sats: " << q1q2_edge_sat << " "
    //           << q2q1_edge_sat << std::endl;

    if (q1q2_edge_sat != 0) {
      m_q_set.edge_saturation()[q1q2]--;
      m_q_set.edge_saturation()[q2q1]--;
    }

    if (q1q2_edge_sat == 0) {
      bool not_sorted = 0;
      // adding adjacent q to update set if their removed edge number exceeds the threshold
      for (unsigned qq = 0; qq < q1qs.size(); qq++) {
        if (q1qs[qq].is_0_valid()) {
          if (1 < qq) {
            if (q1qs[qq][1] < q1qs[qq - 1][1]) {
              not_sorted = true;
            }
          }
          dlm::ikey<3> oq = {reg, q1qs[qq]};
          m_q_set.removed_edge_num()[oq]++;
          if (m_q_set.removed_edge_num()[oq] > removed_edge_update_threshold) {
            if (!m_q_set.to_update_q().is_0_valid(oq)) {
              m_q_set.to_update_q().sublink(oq, oq.cut_end());
            }
            unsigned oc = m_q_set.to_cq()[oq[0]][oq[1]].template get<0>()[oq[2]];
            if (oc != -1) {
              m_color_set.update_color_data()[oq[0]][oc][oq[1]] = true;
            }
          }
        }
      }

      // if (not_sorted || q1[2] == 0) {
      //   if (not_sorted) {
      //     std::cout << "not sorted:" << std::endl;
      //   }
      //   std::cout << "q->oq: " << std::endl;
      //   for (unsigned qq = 0; qq < q1qs.size(); qq++) {
      //     std::cout << q1qs[qq][1] << " ";
      //   }
      //   std::cout << std::endl;
      //   std::cout << "sats: " << std::endl;
      //   for (unsigned qq = 0; qq < q1qs.size(); qq++) {
      //     std::cout << m_q_set.edge_saturation()[q1.add_end(qq)] << " ";
      //   }
      //   std::cout << std::endl;
      // }

      for (unsigned qq = 0; qq < q2qs.size(); qq++) {
        if (q2qs[qq].is_0_valid()) {
          dlm::ikey<3> oq = {reg, q2qs[qq]};
          m_q_set.removed_edge_num()[oq]++;
          if (m_q_set.removed_edge_num()[oq] > removed_edge_update_threshold) {
            if (!m_q_set.to_update_q().is_0_valid(oq)) {
              m_q_set.to_update_q().sublink(oq, oq.cut_end());
            }
            unsigned oc = m_q_set.to_cq()[oq[0]][oq[1]].template get<0>()[oq[2]];
            if (oc != -1) {
              m_color_set.update_color_data()[oq[0]][oc][oq[1]] = true;
            }
          }
        }
      }
      dlm::null(m_q_set, q1q2, q_set::to_qq_n.cut_end());
      dlm::null(m_q_set, q2q1, q_set::to_qq_n.cut_end());
    }

    return q1q2_edge_sat;
  }

  void clear_null_edges() { dlm::clear_null(m_q_set); }

  bool has_edge(unsigned reg, dlm::ikey<2> qt1_q1, dlm::ikey<2> qt2_q2) {
    qt1_q1[0] = qt_to_cqt[qt1_q1[0]];
    qt2_q2[0] = qt_to_cqt[qt2_q2[0]];
    assert(qt1_q1.is_valid());
    assert(qt2_q2.is_valid());

    auto &q1qs = m_q_set.to_qq()[reg][qt1_q1];
    auto &q2qs = m_q_set.to_qq()[reg][qt1_q1];

    // search smaller adjacency
    if (q1qs.size() < q2qs.size()) {
      unsigned dst_index;
      return bin_search(q1qs, qt2_q2, dst_index, 0, q1qs.size() - 1);
    }
  }

  template<unsigned QT>
  const auto &get_color_data(unsigned reg, unsigned c) const {

    static constexpr unsigned cqt = qt_to_cqt[QT];
    static_assert(cqt != -1);
    return get<cqt>(m_color_set.color_data()[reg][c]);
  }

  unsigned color_num(unsigned reg) const {
    return dlm::size(m_color_set, reg, color_set::color_data_n.cut_end());
  };

  void update(unsigned reg) {
    // unlink each q_u's q from its color
    for (unsigned cqt = 0; cqt < cqt_num; cqt++) {
      for (unsigned qu = 0; qu < m_update_q_set.to_q()[reg][cqt].size(); qu++) {
        unsigned q = m_update_q_set.to_q()[reg][cqt][qu];
        m_q_set.to_cq().unlink({reg, cqt, q});
        // clearing null immediately after should be faster.
        dlm::clear_null(m_q_set);
      }
    }
    std::vector<unsigned> &last_seen = m_color_set.last_seen()[reg];
    // update the coloring on all q_u
    for (unsigned cqt = 0; cqt < cqt_num; cqt++) {
      if (m_update_q_set.to_q()[reg][cqt].size() > 0) {
        unsigned s = dlm::size(m_q_set, {reg, cqt}, q_set::to_cq_n.cut_end());
        last_seen.resize(s);
        last_seen.assign(s, -1);
      }

      // greedy coloring
      for (unsigned qu = 0; qu < m_update_q_set.to_q()[reg][cqt].size(); qu++) {
        unsigned q = m_update_q_set.to_q()[reg][cqt][qu];

        // mark colors already in use by neighbors
        const auto &qqs = m_q_set.to_qq()[reg][cqt][q].template get<0>();
        for (unsigned qq = 0; qq < qqs.size(); qq++) {
          dlm::ikey<2> q_other = qqs[qq];
          unsigned c = m_q_set.to_cq()[reg][q_other[0]].template get<0>()[q_other[1]];
          if (c != -1) {
            last_seen[c] = q;
          }
        }

        // find smallest unmarked color
        unsigned c = 0;
        unsigned curr_color_num = dlm::size(m_color_set, {reg}, color_set::color_data_n.cut_end());
        while (c < curr_color_num && last_seen[c] == q) {
          c++;
        }

        if (c == curr_color_num) {
          dlm::grow(m_color_set, 1, {reg}, color_set::color_data_n.cut_end());
        }

        m_q_set.to_cq().sublink(dlm::ikey{reg, cqt, q}, dlm::ikey{reg, c, cqt});
      }
    }

    // mark all color sets the qu are linked to to update
    for (unsigned cqt = 0; cqt < cqt_num; cqt++) {
      for (unsigned qu = 0; qu < m_update_q_set.to_q()[reg][cqt].size(); qu++) {
        unsigned q = m_update_q_set.to_q()[reg][cqt][qu];
        unsigned c = m_q_set.to_cq()[reg][cqt].template get<0>()[q];
        m_color_set.update_color_data()[reg][c][cqt] = true;
      }
    }

    // update color data
    unsigned color_num = dlm::size(m_color_set, {reg}, color_set::color_data_n.cut_end());
    for (unsigned c = 0; c < color_num; c++) {
      for_each_i<cqt_num>([&]<size_t CQT> {
        if (m_color_set.update_color_data()[reg][c][CQT]) {
          const unsigned *cq_to_q = (unsigned *)m_color_set.to_q()[reg][c][CQT].data();
          unsigned cq_num = m_color_set.to_q()[reg][c][CQT].size();
          // for (unsigned cq = 0; cq < cq_num; cq++) {
          //   std::cout << "c: " << c << ", cqt: " << CQT << ", cq: " << cq << ", q: " <<
          //   cq_to_q[cq]
          //             << std::endl;
          // }
          // std::cout << "updating color: " << c << std::endl;

          auto &color_data = get<CQT>(m_color_set.color_data()[reg][c]);
          update_color_data(color_data, cq_to_q, cq_num);
        }
      });
    }

    // clearing
    for (unsigned cqt = 0; cqt < cqt_num; cqt++) {
      dlm::clear(m_update_q_set, {reg, cqt}, update_q_set::to_q_n.cut_end());
    }

    for (unsigned c = 0; c < color_num; c++) {
      for (unsigned cqt = 0; cqt < cqt_num; cqt++) {
        m_color_set.update_color_data()[reg][c][cqt] = false;
      }
    }
  }

  bool check_sorted(unsigned reg) {
    const auto &to_qq = m_q_set.to_qq()[reg];
    for (unsigned cqt = 0; cqt < to_qq.size(); cqt++) {
      for (unsigned q = 0; q < to_qq[cqt].size(); q++) {
        for (unsigned qq = 1; qq < to_qq[cqt][q].size(); qq++) {
          if (to_qq[cqt][q].template get<0>()[qq - 1] > to_qq[cqt][q].template get<0>()[qq]) {
            assert(false);
            return false;
          };
        }
      }
    }
    return true;
  }

private:
  friend class god::state<TW>;

  void init(god::state<TW> &state) {
    m_q_set.relay().init(*this);

    auto &reg_s = get<is_region_state>(state);
    reg_s.add_r1_external(m_q_set);
    reg_s.add_r1_external(m_color_set);
    reg_s.add_r1_external(m_update_q_set);

    for_each_i<cqt_num>([&]<size_t CQT> {
      reg_s.template add_merge_external<cqt_to_qt[CQT]>(m_q_set, q_set::to_cq_n.cut_end(), CQT);
    });

    dlm::init_maps(m_q_set, m_color_set, q_set::to_cq_n, color_set::to_q_n);
    dlm::init_maps(m_q_set, m_update_q_set, q_set::to_update_q_n, update_q_set::to_q_n);
    dlm::init_maps(m_q_set, m_q_set, q_set::to_qq_n, q_set::to_qq_n);
  }

  // clang-format off
  struct q_set:
  dlm::ext<dlm::perm<     // [reg]
    dlm::ext<dlm::perm<   // [cqt]
      dlm::ext<dlm::perm< // [q]
        dlm::comp_ds<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::vec_ds<unsigned char>>>>,
        // [reg][cqt][q] -> [reg][c][cqt][cq]. Stores {c, cq}
        dlm::imap<dlm::merge_key_wrap<
          dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<cqt_num,dlm::split_key_vec<1, 1>>>>,
          0,2,N,1,3  
        >>,
        // [reg][cqt][q] -> [reg][cqt][update_q]. Stores {update_a} 
        dlm::imap<dlm::merge_key_wrap<
          dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::key_vec<1, 1>>>>,
          0,1,N,2
        >>,
        dlm::ssrq<dlm::perm< // [qq]
          //[reg][cqt][q][qq] <=> [reg][cqt][q][qq]. Stores {cqt, q, qq}
          dlm::dmap<dlm::merge_key_wrap<
            dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::vec_ds<dlm::split_key_vec<2, 1>>>>>,
            0,N,N,N,1,2,3
          >>,
          dlm::comp_ds<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::vec_ds<dlm::vec_ds<unsigned>>>>> 
        >>,
        dlm::pointer_relay_ds<color_system_tt<TW>, 3>
      >>
    >>
  >> {
    // a number to represent how many edges have been removed since q was updated
    DEFINE_DS_FIELD((0, 0, 0), removed_edge_num);
    // split due to c and cq often being exclusively accessed
    DEFINE_DS_FIELD((0, 0, 1), to_cq);
    // an inverse map for the subset of qs that need updated
    DEFINE_DS_FIELD((0, 0, 2), to_update_q);
    // {qt, q} split from {qq} due to sorting only needing {qt, q}
    DEFINE_DS_FIELD((0, 0, 3, 0), to_qq);
    // a number to represent how many edges exist between q
    DEFINE_DS_FIELD((0, 0, 3, 1), edge_saturation);
    // a relay to flag color states for updating, resorting qq, and initializing qs to a color
    DEFINE_DS_FIELD((0, 0, 4), relay);
  } m_q_set;

  // clang-format on

  template<unsigned CQT>
  struct color_data_tuple_gen {
    using type = color_data_t<qd_t<cqt_to_qt[CQT], TW>>;
  };

  // clang-format off
  struct color_set:
  dlm::ext<dlm::perm<       // [reg]
    dlm::vec_ds<std::vector<unsigned>>,
    dlm::srq<dlm::perm<     // [c]
      dlm::vec_ds<generated_tuple<cqt_num, color_data_tuple_gen>,2>,
      dlm::perm<   // [cqt]
        dlm::comp_ds<dlm::vec_ds<dlm::vec_ds<dlm::arr_ds<cqt_num, bool>>>>,
        dlm::srq<dlm::perm< // [cq]
          // [reg][c][cqt][cq] -> [reg][cqt][q]. Stores {q}
          dlm::dmap<dlm::merge_key_wrap<
            dlm::comp_key_store<
              dlm::vec_ds<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::key_vec<1, 1>>>>
            >,
            0,N,1,N,2
          >>,
          dlm::pointer_relay_ds<color_set, 4>
        >
      >>
    >>
  >> {
    // detail for greedy algorithm
    DEFINE_DS_FIELD((0), last_seen);
    // user defined color data
    DEFINE_DS_FIELD((1, 0), color_data);
    // flags to tell the update function to update the user defined color data
    DEFINE_DS_FIELD((1, 1, 0), update_color_data);
    // map to q
    DEFINE_DS_FIELD((1, 1, 1, 0), to_q);
    DEFINE_DS_FIELD((1, 1, 1, 1), relay);

    void on_resize(unsigned new_size, dlm::ikey<3> key){
      update_color_data()[key] = true;
    }

    void on_swap(unsigned new_size, dlm::ikey<3> key){
      update_color_data()[key] = true;
    }

    color_set(){
      relay().init(*this);
    }
  } m_color_set;

  // clang-format on

  // clang-format off
  // subset of q who's color needs updated
  struct update_q_set:
  dlm::ext<dlm::perm< // [reg]
    dlm::perm<        // [cqt]
      dlm::perm<      // [q_u]
        // [reg][cqt][q_u] -> [reg][cqt][q]. Stores {q}
        dlm::dmap<dlm::merge_key_wrap<
          dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<cqt_num, dlm::key_vec<1, 1>>>>,
          0, 1, N, 2
        >>
      >
    >
  >> {
    DEFINE_DS_FIELD((0, 0, 0), to_q);
  } m_update_q_set;

  // clang-format on

  // relay functions called whenever [reg][cqt][q] is changed
  friend class dlm::receiver_modifier;

  void on_resize(unsigned new_size, dlm::ikey<2> reg_cqt) {
    unsigned old_size = dlm::size(m_q_set, reg_cqt, q_set::to_update_q_n.cut_end());
    for (unsigned q = old_size; q < new_size; q++) {
      m_q_set.to_update_q().sublink(reg_cqt.add_end(q), reg_cqt);
    }
  }

  void on_swap(dlm::ikey<3> a, dlm::ikey<3> b) {
    // flagging color data for update
    {
      unsigned ca = m_q_set.to_cq()[a.cut_end()].template get<0>()[a.end()];
      if (ca != -1) {
        m_color_set.update_color_data()[a[0]][ca][a[1]] = true;
      }
    }

    {
      unsigned cb = m_q_set.to_cq()[b.cut_end()].template get<0>()[b.end()];
      if (cb != -1) {
        m_color_set.update_color_data()[b[0]][cb][b[1]] = true;
      }
    }
    // reinserting a
    {
      auto &q_to_oq = m_q_set.to_qq()[a].template get<0>();
      auto &q_to_oqq = m_q_set.to_qq()[a].template get<1>();
      for (unsigned q1q = 0; q1q < q_to_oq.size(); q1q++) {
        dlm::ikey<1> reg = a[0];
        dlm::ikey<3> oq = {reg, q_to_oq[q1q]};
        unsigned oqq = q_to_oqq[q1q];

        const auto &oq_to_q = m_q_set.to_qq()[oq].template get<0>();

        unsigned dst_index = oqq;
        bool right_violation = false;
        bool bin_value = false;

        if (oqq < oq_to_q.size() - 1) {
          // violation on the right
          if (oq_to_q[oqq] > oq_to_q[oqq + 1]) {
            bin_value = bin_search(oq_to_q, a.cut_beg(), dst_index, oqq + 1, oq_to_q.size() - 1);
            right_violation = true;
            dst_index--;
          }
        }

        if (oqq > 0 && !right_violation) {
          // violation on the left
          if (oq_to_q[oqq] < oq_to_q[oqq - 1]) {
            bin_value = bin_search(oq_to_q, a.cut_beg(), dst_index, 0, oqq - 1);
          }
        }

        assert(!bin_value && "invalid map");
        // std::cout << "reinsert: " << dst_index << "<-" << oqq << std::endl;
        // simple_print_elements(oq_to_q);
        dlm::ds_modifier::reinsert(m_q_set.to_qq(), oq, dst_index, oqq);
      }
    }

    // reinserting b
    {
      auto &q_to_oq = m_q_set.to_qq()[b].template get<0>();
      auto &q_to_oqq = m_q_set.to_qq()[b].template get<1>();
      for (unsigned q1q = 0; q1q < q_to_oq.size(); q1q++) {
        dlm::ikey<1> reg = b[0];
        dlm::ikey<3> oq = {reg, q_to_oq[q1q]};
        unsigned oqq = q_to_oqq[q1q];

        const auto &oq_to_q = m_q_set.to_qq()[oq].template get<0>();

        unsigned dst_index = oqq;
        bool right_violation = false;
        bool bin_value = false;
        if (oqq < oq_to_q.size() - 1) {
          // violation on the right
          if (oq_to_q[oqq] > oq_to_q[oqq + 1]) {
            bin_value = bin_search(oq_to_q, b.cut_beg(), dst_index, oqq + 1, oq_to_q.size() - 1);
            right_violation = true;
            dst_index--;
          }
        }

        if (oqq > 0 && !right_violation) {
          // violation on the left
          if (oq_to_q[oqq] < oq_to_q[oqq - 1]) {
            bin_value = bin_search(oq_to_q, b.cut_beg(), dst_index, 0, oqq - 1);
          }
        }
        assert(!bin_value && "invalid map");
        // std::cout << "reinsert: " << dst_index << "<-" << oqq << std::endl;
        // simple_print_elements(oq_to_q);

        dlm::ds_modifier::reinsert(m_q_set.to_qq(), oq, dst_index, oqq);
      }
    }
  }

  void on_move(dlm::ikey<3> dst, dlm::ikey<3> src) {
    unsigned c = m_q_set.to_cq()[dst.cut_end()].template get<0>()[dst.end()];
    if (c != -1) {
      m_color_set.update_color_data()[dst[0]][c][dst[1]] = true;
    }
    // this is called after everything else has moved.

    // reinserting src q
    {
      auto &q_to_oq = m_q_set.to_qq()[dst].template get<0>();
      auto &q_to_oqq = m_q_set.to_qq()[dst].template get<1>();
      for (unsigned q1q = 0; q1q < q_to_oq.size(); q1q++) {
        dlm::ikey<1> reg = dst[0];
        dlm::ikey<3> oq = {reg, q_to_oq[q1q]};
        unsigned oqq = q_to_oqq[q1q];

        const auto &oq_to_q = m_q_set.to_qq()[oq].template get<0>();

        // std::cout << "oq to q before:" << std::endl;
        // for (unsigned i = 0; i < oq_to_q.size(); i++) {
        //   std::cout << oq_to_q[i] << " ";
        // }
        // std::cout << std::endl;

        unsigned dst_index = oqq;
        bool right_violation = false;
        bool bin_value = false;
        if (oqq < oq_to_q.size() - 1) {
          // violation on the right
          if (oq_to_q[oqq] > oq_to_q[oqq + 1]) {
            bin_value = bin_search(oq_to_q, dst.cut_beg(), dst_index, oqq + 1, oq_to_q.size() - 1);
            right_violation = true;
            dst_index--;
          }
        }

        if (oqq > 0 && !right_violation) {
          // violation on the left
          if (oq_to_q[oqq] < oq_to_q[oqq - 1]) {
            bin_value = bin_search(oq_to_q, dst.cut_beg(), dst_index, 0, oqq - 1);
          }
        }

        assert(!bin_value && "invalid map");

        dlm::ds_modifier::reinsert(m_q_set.to_qq(), oq, dst_index, oqq);
        // std::cout << "oq to q after:" << std::endl;
        // for (unsigned i = 1; i < oq_to_q.size(); i++) {
        //   if (!(oq_to_q[i - 1] < oq_to_q[i])) {
        //     std::cout << "not sorted!" << std::endl;
        //     for (unsigned i = 0; i < oq_to_q.size(); i++) {
        //       std::cout << oq_to_q[i] << " ";
        //     }
        //     assert(false);
        //   };
        // }
        // std::cout << std::endl;

        // unsigned oc = m_q_set.to_cq()[oq.cut_end()].template get<0>()[oq.end()];
        // if (oc != -1) {
        //   m_color_set.update_color_data()[oq[0]][oc][oq[1]] = true;
        // }
      }
    }
  }
};

struct color_system {
  template<typename TW>
  using template_t = color_system_tt<TW>;
};
} // namespace cp6

/*
  THOUGHTS:
    Our current q_set map is flawwed
      Right now it's:
        [reg][cqt][q]
      But to work with regions needs to be
        [cqt][reg][q]

      I think we could make a perm map that has the behavior... but then agian
      this way sort reflects how perms actually are.

      Then again, an insertion method could lead to much more flexible and intuitive index
      structures.

      dlm::insert_perm_map<2, 0, 1> inserts the 0th index into the 1st index of the rt key.

      I could also make a custom type just for the region system since this isn't user facing. But
      the functions are only accesable to perm map. I could modify it so there are insertion numbers
      at the end that default to it's current behavior, however insertion is complecated and would
  require me to pull in the insertion functions from pkey. I would want to refomat these functions
  slightly so that the insertion is from right to left, but this would require modification of
  everything that uses index_set_v5 as a dependency, so I should make it v6.
        //[reg]<qt>[q] -> [reg][qt][q]
        dlm::perm_map<2, 0, 1> inserts 0th index of the perm key into the 1st index of the input
  key. The current behavior can be retained like dlm::perm_map<2, 0, 0> inserts the 0th index of the
  perm key into the 0th index of the input key.

      Index set v6 will have first class insertion support.

    Resorting
      To maintain the sorted guarantee of the adjacency, we need to
      resort whenever a q's qq moves.

      However, doing this without breaking recursion rules is difficult.

      The easiest way is to simply mark the q as needing resorted and then
      whenever add or remove edge is called it gets done. T

      We could also take advantage of the fact that nothing is aligned with rank4
      to_qq set of the to_qq set and use ds_modifier operations to resort
      whenever a q moves.

    Depedency stuff
      Only qts that are marked colorable be colored.
      Only qt-qt pairs that are Gauss Siedel should have color edges.
      The most straight forward way to achieve this is to have the color
      system be in the core library and have the submodules be in control of adding
      and removing the right edges.

      What are the edge rules for pcnt?
        If qt-qt is marked as contributing to the colors is the most general way, so
        we could just have a qt based flag. ENABLE_PCNT_COLOR_EDGE(qt1, qt2);


    Let's go with the central q-q map approach.
      we need the following sets and maps:
        [reg][c]<cqt>(color_data)
        [reg][c][cqt][cq]
        [reg][cqt][q][qq]
        [reg][cqt][update_q]

        [reg][cqt][q][qq] <=> [reg][cqt][q][qq]
        [reg][cqt][q] <=> [reg][c][cqt][cq]
        [reg][cqt][update_q] -> [reg][cqt][q]

    Persistent coloring persists accross frames and is only updated when an edge changes.
      This can be achieved, but only for persistant contacts and persistent constraints, which are
      really our main two things that contribute to coloring. Non-persistant contacts usually
  belong to coordinate types that don't need coloring, and other forces are jacobi.

      This requires a standard for edges accross systems that contribute them.

      An edge across coord types is usually stored in a set like this [reg]<qt1, qt2>[e] with
  certain qt-qt pairs cut off. Edges can also be stored in a set like this [reg][qt][e], but this
  is uncommon.

      However, we probably want to do all this color update stuff at once to avoid it
  bottlenecking other tasks.

      Since we are doing greedy coloring, all we need is to mark which q needs updated. This
  requires perfect support from other systems to do, but shouldn't be too hard to implement.

      Once we have our set of q's that need to be updated, we need a common way to get their
  adjacencies.

    Central q-q edge state.
      let's create a set to store q-q edges:
        [reg][qt][q][qq]
      We can align colors with the first 3 ranks of this set

      Other states could easily control this through the add_edge and remove_edge functions.

      The only thing is that q_q would have to be searched through to add or remove an edge.
      Having it sorted could help with this.

      If we add the constraint of all edges adding a phaselink, then the search would only be
  required when removing an edge. But this would limit stuff.

      Since the search is only needed when adding/removing/checking for an edge and since the
  operations are relatively infrequent, I think this is a viable option. This isn't like
  phaselinks where the search would have to be done multiple times each frame. It is probably best
  to limit this set to use by systems that contribute to colors. Having this as a general q-q edge
  state will just make this set less usefull for coloring.



    How tf is coloring going to work?
      After testing stuff out, doing it only through edges is wierd.
      It works, but it need multiple iterations and it produces more colors than just a base greed
  approach.

      The problem with the greedy approach though is that it requires an adjacency list between
  all qs to be generated from all edges across all edge contributing states.

      The edge only version could be built by just gathering all the edge states and doing the
  algorithim on them.

      The q adjacency can be built from edge states. It is rank 3, [qt][q][qq].
*/

//
// template<typename... CQTs>
// class color_system_state;
//
// // clang-format off
// template<typename...CQTs>
// struct color_system_q_set:
// dlm::ext<dlm::perm<     // [reg]
//   dlm::ext<dlm::perm<   // [cqt]
//     dlm::ext<dlm::perm< // [q]
//       dlm::pointer_relay_ds<color_system_state<CQTW.>,
//       // [reg][cqt][q] -> [reg][c][cqt][cq]
//       dlm::imap<dlm::pkey_wrap<
//         dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<sizeof...(CQTWdlm::split_key_vec<1, 1>>>>,
//         0,0, // [reg] <- [reg]
//         2,1  // [cqt] <- [cqt]
//       >>,
//       // [reg][cqt][q] -> [reg][cqt][update_q]
//       dlm::imap<dlm::pkey_wrap<
//         dlm::comp_key_store<
//           dlm::vec_ds<dlm::arr_ds<sizeof...(CQTWdlm::key_vec<1, 1>>>
//         >,
//         0,0, // [reg] <- [reg]
//         1,1  // [cqt] <- [cqt]
//       >>,
//       dlm::vec_ds<dlm::arr_ds<sizeof...(CQTWdlm::vec_ds<unsigned char>>>,
//       dlm::ssrq<dlm::perm< // [qq]
//         //[reg][cqt][q][qq] <=> [reg][cqt][q][qq]
//         dlm::dmap<dlm::pkey_wrap<
//           dlm::comp_key_store<dlm::vec_ds<dlm::arr_ds<sizeof...(CQTs),
//           dlm::vec_ds<dlm::key_vec<1, 2>>>>>, 0,0, // [reg] <- [reg] 1,1  // [cqt] <- [cqt]
//         >>,
//         dlm::comp_ds<dlm::vec_ds<dlm::arr_ds<sizeof...(CQTs),
//         dlm::vec_ds<dlm::vec_ds<unsigned>>>>>
//       >>
//     >>
//   >>
// >> {
//   // a relay to flag color states for updating and for resorting qq
//   DEFINE_DS_FIELD((0, 0, 0), relay);
//   // split due to c and cq often being exclusively accessed
//   DEFINE_DS_FIELD((0, 0, 1), to_cq);
//   // an inverse map for the subset of qs that need updated
//   DEFINE_DS_FIELD((0, 0, 2), to_update_q);
//   // a number to represent how many edges have been removed since q was updated
//   DEFINE_DS_FIELD((0, 0, 3), removed_edge_num);
//   // not split due to sorting using the entire key
//   DEFINE_DS_FIELD((0, 0, 4, 0), to_qq);
//   // a number to represent how many edges exist between q
//   DEFINE_DS_FIELD((0, 0, 5, 1), edge_saturation);
// };
//
// // clang-format on
//
// // clang-format off
// template<typename...CQTs>
// struct color_system_color_set:
// dlm::ext<dlm::perm<       // [reg]
//   dlm::vec_ds<std::vector<unsigned>>,
//   dlm::ext<dlm::perm<     // [c]
//     dlm::vec_ds<std::tuple<color_data_t<CQTW.>,2>,
//     dlm::ext<dlm::perm<   // [cqt]
//       dlm::comp_ds<dlm::vec_ds<dlm::vec_ds<dlm::arr_ds<sizeof...(CQTWbool>>>>,
//       dlm::srq<dlm::perm< // [cq]
//         // [reg][c][cqt][cq] -> [reg][cqt][q]
//         dlm::dmap<dlm::pkey_wrap<
//           dlm::comp_key_store<
//             dlm::vec_ds<dlm::vec_ds<dlm::arr_ds<sizeof...(CQTWdlm::split_key_vec<1, 1>>>>
//           >,
//           0,0, // [reg] <- [reg]
//           1,2  // [cqt] <- [cqt]
//         >>
//       >>
//     >>
//   >>
// >> {
//   // detail for greedy algorithm
//   DEFINE_DS_FIELD((0), last_seen);
//   // user defined color data
//   DEFINE_DS_FIELD((1, 0), color_data);
//   // flags to tell the update function to update the user defined color data
//   DEFINE_DS_FIELD((1, 1, 0), update_color_data);
//   // map to q
//   DEFINE_DS_FIELD((1, 1, 1, 0), to_q);
// };
//
// // clang-format on
//
// template<typename... CQTs>
// struct color_system_state<std::tuple<CQTW>> {
//   using q_set_t = color_system_q_set<CQTW>;
//   using color_set_t = color_system_color_set<CQTW>;
//   q_set_t m_q_set;
//   color_set_t m_color_set;
//
//   color_system_state() { m_q_set.relay().init(*this); }
//
//   void on_resize(unsigned new_size, dlm::ikey<3> q) {}
//
//   void on_swap(dlm::ikey<3> q1, dlm::ikey<3> q2) {
//     unsigned dst_c = m_q_set.to_cq()[q1.cut_end()].template get<0>()[q1.end()];
//     unsigned src_c = m_q_set.to_cq()[q2.cut_end()].template get<0>()[q2.end()];
//
//     m_color_set.update_color_data()[q1[0]][dst_c][q1[0]] = true;
//     m_color_set.update_color_data()[q2[0]][src_c][q2[0]] = true;
//
//     // this is called before the maps are updated. This means we must
//     // sort the qq's adjacencies to what they will be.
//     //
//     // for qq in q.qq:
//     //    reinsert()
//   }
//
//   void on_move(dlm::ikey<3> dst, dlm::ikey<3> src) {
//     unsigned c = m_q_set.to_cq()[src.cut_end()].template get<0>()[src.end()];
//     if (c != -1) {
//       m_color_set.update_color_data()[src[0]][c][src[1]] = true;
//     }
//   }
//
//   void on_null(dlm::ikey<3> q) {
//     unsigned c = m_q_set.to_cq()[q.cut_end()].template get<0>()[q.end()];
//     if (c != -1) {
//       m_color_set.update_color_data()[q[0]][c][q[1]] = true;
//     }
//   }
// };
