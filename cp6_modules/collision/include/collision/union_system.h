#pragma once
#include "arr_ds.h"
#include "comp_ds.h"
#include "god_state.h"
#include "ds_utils.h"
#include "external_perm.h"
#include "ikey.h"
#include "index_maps/base_map.h"
#include "index_maps/comp_key_store.h"
#include "index_maps/dmap.h"
#include "index_maps/imap.h"
#include "index_maps/key_r2_coherent.h"
#include "index_maps/pkey_wrap.h"
#include "index_maps/split_key_vec.h"
#include "maths.h"
#include "operation_fn_ptrs.h"
#include "perm.h"
#include "region_state.h"
#include "relay/receiver_modifier.h"
#include "tuple_utils.h"
#include <relay/pointer_relay_ds.h>

namespace cp6 {
template<typename ROOT_QT>
struct is_union_child {
  template<typename T>
  struct pred {
    static constexpr bool value =
      requires { requires(std::is_same_v<typename T::root_qt, ROOT_QT>); };
  };
};

// index sets:
// [reg][rq][cqt][child_q]
// [cqt][reg][q]
//
// maps:
//
//[reg][rq][cqt][rq_cq] Stores [q]
//<=>
//[cqt][reg][cq] Stores [rq][rq_cq]
template<typename ROOT_QT, typename TW>
struct union_system_tt {
  using rqt_t = ROOT_QT;
  template<typename T>
  using child_pred = typename is_union_child<ROOT_QT>::template pred<T>;

  static constexpr unsigned cqt_num = get_matching_num<child_pred, cp6::qt_ts<TW>>();
  static constexpr auto qt_to_cqt =
    generate_tuple_indirection<cp6::qt_ts<TW>, get_matching_ts<child_pred, cp6::qt_ts<TW>>>();
  static constexpr auto cqt_to_qt =
    generate_tuple_indirection<get_matching_ts<child_pred, cp6::qt_ts<TW>>, cp6::qt_ts<TW>>();
  static constexpr unsigned rqt = cp6::get_qt_index<ROOT_QT, TW>();

  // [cq]
  const auto &to_rq(unsigned reg, unsigned qt) {
    assert(qt_to_cqt[qt] != -1);
    return m_cq_data.to_rq_cq()[qt_to_cqt[qt]][reg].template get<0>();
  }

  // [cq]
  const auto &to_rq_cq(unsigned reg, unsigned qt) {
    assert(qt_to_cqt[qt] != -1);
    return m_cq_data.to_rq_cq()[qt_to_cqt[qt]][reg].template get<1>();
  }

  // [rq][cqt][rq_cq]
  const auto &to_cq(unsigned reg, unsigned qt) { return m_rq_data.to_cq()[reg]; }

  void link_to_root(unsigned reg, unsigned rq, unsigned child_qt, unsigned cq) {
    assert(qt_to_cqt[child_qt] != -1);
    m_cq_data.to_rq_cq().sublink(
      dlm::ikey{qt_to_cqt[child_qt], reg, cq},
      dlm::ikey{reg, rq, qt_to_cqt[child_qt]}
    );
  }

  // [cq]
  vec2f *body_translation(unsigned reg, unsigned child_qt) {
    assert(qt_to_cqt[child_qt] != -1);
    return m_cq_data.body_translation()[qt_to_cqt[child_qt]][reg].data();
  }

  // [cq]
  float *body_rotation(unsigned reg, unsigned child_qt) {
    assert(qt_to_cqt[child_qt] != -1);
    return m_cq_data.body_rotation()[qt_to_cqt[child_qt]][reg].data();
  }

  template<unsigned QT_A, unsigned QT_B>
  bool can_collide(unsigned reg, unsigned q_a, unsigned q_b) {
    static_assert(qt_to_cqt[QT_A] != -1 && qt_to_cqt[QT_B] != -1);
    return to_rq(reg, QT_A)[q_a] != to_rq(reg, QT_B)[q_b];
  }

private:
  friend class dlm::receiver_modifier;
  friend class god::state<TW>;

  void init(god::state<TW> &state) {
    m_state = &state;
    auto &region_s = get<is_region_state>(state);

    dlm::init_maps(m_rq_data, m_cq_data, m_rq_data.to_cq_n, m_cq_data.to_rq_cq_n);
    region_s.add_r1_external(m_rq_data);
    region_s.template add_r2_external<rqt>(m_rq_data, dlm::ckey<0>());
    for_each_i<cqt_num>([&]<unsigned CQT> {
      region_s.add_r11_external(m_cq_data, dlm::ckey<0>{}, CQT);

      region_s
        .template add_r21_external<cqt_to_qt[CQT]>(m_cq_data, m_cq_data.to_rq_cq_n.cut_end(), CQT);
    });

    m_rq_data.self_pointer().init(*this);
    m_region_state = &region_s;
    m_rqt_q_null_fn = dlm::get_null_fn(region_s, dlm::ckey<rqt>{});
  }

  // dependency info. By default, roots are deleted when all of their children are.
  struct dinfo {
    unsigned child_num;
    unsigned nulled_child_num;
  };

  // [reg][rq][cqt][rq_cq]
  struct root_data:
  dlm::ext<dlm::perm< // [reg]
    dlm::ext<dlm::perm< // [rq]
      dlm::perm< // [cqt]
        dlm::perm< // [rq_cq]
          //[reg][rq][cqt][rq_cq] <=> [cqt][reg][cq]. Stores [cq]
          dlm::dmap<dlm::pkey_wrap<dlm::comp_key_store<
            dlm::vec_ds<dlm::vec_ds<dlm::key_r2_coherent<cqt_num, 1>>>
          >, 0, 2, 0, 0>>,
          dlm::pointer_relay_ds<union_system_tt, 4>
        > 
       >,
      dlm::vec_ds<dinfo, 2>
    >>
  >>
  {
    DEFINE_DS_FIELD((0, 1), dinfos);
    DEFINE_DS_FIELD((0, 0, 0, 0), to_cq);
    DEFINE_DS_FIELD((0, 0, 0, 1), self_pointer);
  } m_rq_data;

  template<rank_t KEY_RANK>
  void on_null(dlm::ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == 4) {
      dinfo &info = m_rq_data.dinfos()[key.template shrink_to_beg<2>()];
      info.nulled_child_num++;
      if (info.nulled_child_num == info.child_num) {
        // dlm::is_vtable::null(m_region_state_vkey, dlm::ikey<1>{rqt}, key.template
        // shrink_to_beg<2>());
        m_rqt_q_null_fn(m_region_state, key.template shrink_to_beg<2>());
      }
    }
  }

  template<rank_t KEY_RANK>
  void on_resize(unsigned new_size, dlm::ikey<KEY_RANK> key) {
    if constexpr (KEY_RANK == 3) {
      dinfo &info = m_rq_data.dinfos()[key.template shrink_to_beg<2>()];
      int diff = (int)new_size - (int)size(m_rq_data, key, dlm::ckey<0, 0, 0>{});
      m_rq_data.dinfos()[key.template shrink_to_beg<2>()].child_num += diff;
    }
  }

  // [cqt][reg][cq]
  struct child_data:
  dlm::ext<dlm::perm<     // [cqt]
    dlm::ext<dlm::perm<   // [reg]
      dlm::ext<dlm::perm< // [cq]
                          //[cqt][reg][cq] <=> [reg][rq][cqt][rq_cq]. Stores [rq][rq_cq]
        dlm::imap<dlm::pkey_wrap<
          dlm::comp_key_store<dlm::arr_ds<cqt_num, dlm::vec_ds<dlm::split_key_vec<1, 1>>>>,
          0,
          1,
          1,
          0
        >>,
        dlm::comp_ds<dlm::arr_ds<cqt_num, dlm::vec_ds<vec2f, 2>>>,
        dlm::comp_ds<dlm::arr_ds<cqt_num, dlm::vec_ds<float, 2>>>
      >>
    >>
  >> {
    DEFINE_DS_FIELD((0, 0, 0), to_rq_cq);
    DEFINE_DS_FIELD((0, 0, 1), body_translation);
    DEFINE_DS_FIELD((0, 0, 2), body_rotation);
  } m_cq_data;

  god::state<TW> *m_state;
  dlm::null_fn<2> m_rqt_q_null_fn;
  void *m_region_state;
  // dlm::is_vkey m_region_state_vkey;
};

template<typename ID>
struct union_system {
  template<typename TW>
  using template_t = union_system_tt<ID, TW>;
};
} // namespace cp6

/*
  THOUGHTS:

  How should child_qts link back to the root?
    We are currently using types, but there is no gaurentee that each qt
    will be unique. However, there is no benifit to having multiple of the
    exact same coordinate type. The same effect could be achieved by growing
    the first. If specialization is to be made, let it be on the type level.


  Should we seperate collider data into it's own set.
    Now that we relationship between coordinate types and collider data
    that isn't one to one, it makes a little sense to have these in seperate sets.

    The concept of phase sets, layers, phase links, and unions all only effect the colliders
    of qts. The reason coordinates and colliders are bundled in the first place is to update
    the masses of the coordinate. However, this can be done even if the masses are outside the
    set.

    Seperating collider data from coordiante types allows for collider data without coordinates
    and coordintates with multiple colliders. It also distills the purpose of both systems:
    qts hold coordinates and cd holds data needed for collision.

    index sets:
      [reg]<qt>[q]
      [reg]<cd>[c]

    The hard part is defining maps arbitrarilly. Some maps are simply defined as <qt>[q] == <cd>[c],
  while others, like unions, are defined using a many colldier to one coordinate system. Also, some
  collider datas wont even have an associate qt. Tags, once again, are our ticket to achieving this.
  Each cd can have a tag that tells a system to align data with the cd. single_cd qt_type multi_cd
          qt_type
        (nothing) == no_qt

    What benifits does this have compared to just having the collider data share the same qt index?
      Currently each qt must have a cd() function.  We could remove this necessity.
      Qts that have no colliders wont waste space in the cd only systems

      Sure this is nice, but does it provide functionality that cannot be achieved with the current
  system? The cd() function could return a dummy type for qts that don't want to collide. Wasting
  space provides no downside to runtime performance. Multiple colliders can be set up using qts that
  only contain collider data.

        Benifits to the current way.
          Conveient one to one mapping between c and q for coordinates that need it.
          A single set that all systems use.





  How will child/root dependency work.
    The child coordiante types are basically glorified collider data that can
    be a part of layers and phase sets. Before now, we haven't really had the need
    to have coordinate types dependent on each other.

    This dependency can really be handled by whatever is removing / adding coordinate types.
    This implies that qs can only be added/removed by systems outside the cross state. This
    constraint sorta makes sense.

  How will unions work?
    Only certian qts can be the root of a union (rigid bodies affine bodies)

    Only certain qts can be children of a union (you don't want a liquid particle to be part of a
    union).

    This means there are two sets: union_rqts and union child qts. And a map between them:
    [reg][rqt][q][cqt][q_q] <=> [reg][cqt][q]

    The mostly unchanging nature of a union means that it is best to use a coherent key storage.

    Unions indirect the forces from the children to the parent.

    However, 99% of use cases are covered if we limit child qs to only link to root qs in the same
    coord type.

    To reduce indirection, we can have qts that always are child q and qts that are always root q.
      How are we supposed to connect these?
        We could use the raw qt indices... but that's not very generalizable.
        We could use an ID system like we did with layers.
          region_state<union_qt<qt_type0, id_type>, other_qt, child_qt<qt_type1, id_type>,
          child_qt<qt_type2, id_type>>

        The union system can then look for the tag 'union_root' and then use the id type to find
        the children.

        The children only need the type of the root as an id.
          region_state<rqt, child_qt<rqt>, child_qt<rqt>>



      Can a union child be apart of multiple union roots?
        I currently don't see the benifit in allowing this. If this was allowed, every child qt
        would be split based on what root their qs went to, this would require some dynamic
        dispatch to take care of. Just have multiple sets, each going to one root.

      These children coord types need to act more as external collider data rather than a full
  fledged coordtype.

      The main map that will be used is the one from the child to the root. But how do we get from
  the child qt to the root qt? We could store a qt_to_child qt in the union that the child qt can
  use to get it's map. get<union_system<id>>(state).cq_to_rq<qt>();

        We could change the standard and allow the child qt to take in template arguments, so this
        map is internal.
          get<child_qt>(state).cq_to_rq();

  Should we change the standard to allow qts to take in template arguments?
    Doing so would allow for qt data to be stored localy inside the coordinate type.
    However this comes at the cost of longer qt type signitures. However, this probably
    doesn't matter. The coord type can become more modularized, having fields like vbd_data
    along side it's coordinate data.

    However, as of now, the systems and state classes can align any state that is needed to a qt
    and there hasn't been a sitution where a dependency couldn't be injected by another system.
    In fact, if we did the fields, the dependencies would have to be injected from the qt.

    qt.member == system.qt_data.member

    What is a qt?
      A qt is an index set that aligns with a single coordinate.


  How will lambda be stored in two point persistant contacts?
    Create a custom constraint type that is aligned with the persistant collision set.

    NVM:
        Indirections can also be made from the persistant contact set to other sets. So,
        some thing like this could be made:
          [pcnt(contact a)] <=> [solver_contact]
          [pcnt(contact b)] <=> [solver_contact]
        Since the solver to pcnt map is only used as an inverse, it doesn't matter that it
        has a lot of invalid indices.

        There wouldn't be a good way to remove the second contact when it became invalid.


  Will a child's persistant contacts be redirected to the root?
    No, this isn't necessary. The collision resolution system can take into account
    the indirection and effect the correct q. How it does this is up to the system.

    Also, removing the perstant contact of the child q would result in it not being
    able to add a phase link.
*/
