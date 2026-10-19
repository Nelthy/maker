#pragma once
#include "god_state.h"
#include "ds_utils.h"
#include "external_perm.h"
#include "ikey.h"
#include "index_maps/base_map.h"
#include "index_maps/dmap.h"
#include "index_maps/imap.h"
#include "index_maps/key_vec.h"
#include "index_maps/pkey_wrap.h"
#include "layer_state.h"
#include "perm.h"
#include "set_remove_queue.h"

namespace cp6 {
template<typename... Ts>
struct sub_layer_system_tt {

  template<unsigned QT_A, unsigned QT_B>
  bool can_collide(unsigned reg, unsigned q_a, unsigned q_b) {
    auto &layer_s = get<layer_state>(*m_state);

    dlm::ikey<2> ls_lay_a = layer_s.template q_to_lay_q<QT_A>()[reg].template get<0>()[q_a];
    dlm::ikey<2> ls_lay_b = layer_s.template q_to_lay_q<QT_B>()[reg].template get<0>()[q_b];

    // if one of the qs aren't apart of a layer
    if (!ls_lay_a.is_0_valid() || !ls_lay_b.is_0_valid()) {
      return true;
    }

    // if both layers are the same.
    if (ls_lay_a == ls_lay_b) {
      return true;
    }

    // if both qs have different roots
    if (m_layer_data.root()[reg][ls_lay_a[0]] != m_layer_data.root()[reg][ls_lay_b[0]]) {
      return true;
    }

    // get ancestors with the same distance to the root
    unsigned root_dist_a = m_layer_data.root_dist()[reg][ls_lay_a[0]];
    unsigned root_dist_b = m_layer_data.root_dist()[reg][ls_lay_b[0]];

    if (root_dist_a < root_dist_b) {
      for (unsigned i = 0; i < root_dist_b - root_dist_a; i++) {
        ls_lay_b = m_layer_data.to_lay_ls()[reg][ls_lay_b[0]].cut_end();
      }
    } else {
      for (unsigned i = 0; i < root_dist_a - root_dist_b; i++) {
        ls_lay_a = m_layer_data.to_lay_ls()[reg][ls_lay_a[0]].cut_end();
      }
      root_dist_a = root_dist_b;
    }
    if (ls_lay_a[0] == ls_lay_b[0]) {
      if (ls_lay_a[1] == ls_lay_b[1]) {
        return true;
      } else {
        return false;
      }
    }

    // go up towards the root until a common ancestor is found
    for (unsigned i = 0; i < root_dist_a; i++) {
      if (ls_lay_a[0] == ls_lay_b[0]) {
        if (ls_lay_a[1] == ls_lay_b[1]) {
          return true;
        } else {
          return false;
        }
      }
      ls_lay_a = m_layer_data.to_lay_ls()[reg][ls_lay_a[0]].cut_end();
      ls_lay_b = m_layer_data.to_lay_ls()[reg][ls_lay_b[0]].cut_end();
    }

    return false;
  }

  // [reg][ls] <=> [reg][ls][lay][lay_ls]. Stores [ls][lay][lay_ls]
  const auto &to_lay_ls() const { return m_layer_data.to_lay_ls(); }

  // [reg][ls][lay][lay_ls] <=> [reg][ls]. Stores [ls]
  const auto &to_ls() const { return m_layer_data.to_ls(); }

  void add_sublayer(dlm::ikey<1> reg, dlm::ikey<2> ls_lay, dlm::ikey<1> sub_ls) {
    m_layer_data.to_lay_ls().sublink(dlm::ikey{reg, sub_ls}, dlm::ikey{reg, ls_lay});
    update_root_data({reg, sub_ls});
  }

  void remove_from_sublayer(dlm::ikey<2> reg_ls) {
    m_layer_data.to_lay_ls().unlink(reg_ls);
    m_layer_data.clear_null();
    update_root_data(reg_ls);
  }

  void remove_sublayer(dlm::ikey<4> reg_ls_lay_lay_ls) {
    dlm::ikey<1> ls = m_layer_data.to_ls()[reg_ls_lay_lay_ls];
    m_layer_data.to_ls().unlink(reg_ls_lay_lay_ls);
    m_layer_data.clear_null();
    update_root_data({reg_ls_lay_lay_ls[0], ls});
  }

private:
  friend class god::state<Ts...>;

  void update_root_data(dlm::ikey<2> reg_ls) {
    // getting the root, reg_ls must be a child
    dlm::ikey<2> root = m_layer_data.to_lay_ls()[reg_ls].cut_end();
    while (m_layer_data.to_lay_ls()[root].is_0_valid()) {
      root = m_layer_data.to_lay_ls()[reg_ls].cut_end();
    }
    propagate_root_data(root, root.cut_end());
  }

  void propagate_root_data(dlm::ikey<2> reg_ls, const unsigned &root, unsigned dist = 0) {
    m_layer_data.root()[reg_ls] = root;
    m_layer_data.root_dist()[reg_ls] = dist;
    for (unsigned lay = 0; lay < size<0, 3>(m_layer_data, reg_ls); lay++) {
      for (unsigned lay_ls = 0; lay_ls < size<0, 3, 0>(m_layer_data, reg_ls.add_end(lay));
           lay_ls++) {
        propagate_root_data(
          m_layer_data.to_ls().get_inverse(reg_ls.add_end(lay).add_end(lay_ls)),
          root,
          dist + 1
        );
      }
    }
  }

  void init(god::state<Ts...> &state) {
    m_state = &state;

    dlm::init_maps(m_layer_data, m_layer_data, m_layer_data.to_ls_n, m_layer_data.to_lay_ls_n);

    get<is_region_state>(state).add_r1_external(m_layer_data);
    get<layer_state>(state).m_layer_data.add_r2_external(m_layer_data, dlm::ckey<0>{});
    get<layer_state>(state).m_layer_data.add_r3_external(m_layer_data, dlm::ckey<0, 3>{});
  }

  // maps:
  // [reg][ls] <=> [reg][ls][lay][lay_ls]

  // [reg][ls][lay][lay_ls]
  struct layer_set:
  dlm::ext<dlm::perm<   // [reg]
    dlm::ext<dlm::perm< // [ls]
                        // [reg][ls] <=> [reg][ls][lay][lay_ls]. Stores [ls][lay][lay_ls]
      dlm::imap<dlm::pkey_wrap<dlm::key_vec<2, 3>, 0, 0>>,
      dlm::vec_ds<unsigned, 2>,
      dlm::vec_ds<unsigned, 2>,
      dlm::ext<dlm::perm<   // [lay]
        dlm::srq<dlm::perm< // [lay_ls]
                            // [reg][ls][lay][lay_ls] <=> [reg][ls]. Stores [ls]
          dlm::dmap<dlm::pkey_wrap<dlm::key_vec<4, 1>, 0, 0>>
        >>
      >>
    >>
  >> {
    DEFINE_DS_FIELD((0, 0), to_lay_ls);
    DEFINE_DS_FIELD((0, 1), root);
    DEFINE_DS_FIELD((0, 2), root_dist);
    DEFINE_DS_FIELD((0, 3, 0, 0), to_ls);
  } m_layer_data;

  god::state<Ts...> *m_state;
};

struct sub_layer_system {

  template<typename... Ts>
  using template_t = sub_layer_system_tt<Ts...>;
};
} // namespace cp6
