#pragma once
#include "ckey.h"
#include "comp_ds.h"
#include "god_state.h"
#include "ds_utils.h"
#include "external_perm.h"
#include "index_maps/base_map.h"
#include "index_maps/comp_key_store.h"
#include "index_maps/dmap.h"
#include "index_maps/imap.h"
#include "index_maps/key_vec.h"
#include "index_maps/pkey_wrap.h"
#include "index_maps/split_key_vec.h"
#include "perm.h"
#include "perm_arr.h"
#include "region_state.h"
#include "set_remove_queue.h"
#include "vec_bundle_ds.h"
#include "vec_ds.h"

namespace cp6 {
template<typename... Ts>
struct layer_state_tt {
  static constexpr unsigned qt_num = get_qt_num<Ts...>();

  // helper functions
  // [reg][ls][lay][lay_q]
  template<unsigned QT>
  const auto &lay_q_to_q() const {
    return m_layer_data.template to_q<QT>();
  }

  // [reg][q]. get<0> for {ls, lay}. get<1> for {lay_q}
  template<unsigned QT>
  const auto &q_to_lay_q() const {
    return m_q_data.template to_lay_q<QT>();
  }

  template<unsigned QT_A, unsigned QT_B>
  bool can_collide(unsigned reg, unsigned q_a, unsigned q_b) const {
    dlm::ikey<2> ls_lay_a = m_q_data.template to_lay_q<QT_A>()[reg].template get<0>()[q_a];
    dlm::ikey<2> ls_lay_b = m_q_data.template to_lay_q<QT_B>()[reg].template get<0>()[q_b];
    if (!ls_lay_a.is_0_valid() || !ls_lay_b.is_0_valid()) {
      return true;
    }

    if (ls_lay_a[0] != ls_lay_b[0]) {
      return true;
    }

    if (ls_lay_a[1] == ls_lay_b[1]) {
      return true;
    }

    return false;
  }

  // [reg][ls][lay]<qt>[lay_q]
  struct layer_set:
  dlm::ext<dlm::perm<   // [reg]
    dlm::srq<dlm::perm< // [ls]
      dlm::perm_arr< // needs special remove queue
        qt_num,
        dlm::srq<dlm::perm< // <qt>[lay_q]
          // [reg][ls][lay]<qt>[lay_q] <=> [reg]<qt>[q]. Stores [q]
          dlm::dmap<dlm::pkey_wrap<dlm::key_vec<4, 1>, 0, 0>>,
          dlm::vec_bundle_ds<dlm::perm_map<4>>
        >>,
        dlm::vec_bundle_ds<dlm::perm_map<3>>
      >,
      dlm::vec_bundle_ds<dlm::perm_map<2>>
    >>
  >> {
    template<unsigned QT>
    auto &to_q() {
      return get<0, 0, QT, 0>(*this);
    }

    template<unsigned QT>
    const auto &to_q() const {
      return this->template get<0, 0, QT, 0>();
    }

    // [reg][ls]
    template<typename IS, rank_t P>
    void add_r2_external(IS &is, dlm::ckey<P> perm) {
      get<0, 1>(*this).add_ds(dlm::perm_map<2>{is, perm});
    }

    // [reg][ls][lay]
    template<typename IS, rank_t P1, rank_t P2>
    void add_r3_external(IS &is, dlm::ckey<P1, P2> perm) {
      get<0, 0, qt_num>(*this).add_ds(dlm::perm_map<3>{is, perm});
    }

    // [reg][ls][lay]<qt>[lay_q]
    template<unsigned QT, typename IS, rank_t P1, rank_t P2, rank_t P3>
    void add_r4_external(IS &is, dlm::ckey<P1, P2, P3> perm) {
      get<0, 0, QT, 1>(*this).add_ds(dlm::perm_map<4>{is, perm});
    }
  } m_layer_data;

  struct q_set:
  dlm::ext<dlm::perm_arr< // [reg]
    qt_num,
    dlm::ext<dlm::perm< // <qt>[q]
      // [reg]<qt>[q] <=> [reg][ls][lay]<qt>[lay_q]. Stores [ls][lay][lay_q]
      dlm::imap<dlm::pkey_wrap<dlm::comp_key_store<
        dlm::vec_ds<dlm::split_key_vec<2, 1>>
      >, 0, 0>>
    >>
  >> {
    template<unsigned QT>
    auto &to_lay_q() {
      return get<QT, 0>(this);
    }

    template<unsigned QT>
    const auto &to_lay_q() const {
      return get<QT, 0>(*this);
    }
  } m_q_data;

private:
  friend class god::state<Ts...>;

  // index sets:
  // [reg][ls][lay]<qt>[lay_q]
  // [reg][ls][lay][lay_lay]
  //
  // maps:
  // [reg]<qt>[q] <=> [reg][ls][lay]<qt>[lay_q]

  void init(god::state<Ts...> &state) {
    m_state = &state;

    get<is_region_state>(state).add_r1_external(m_q_data);
    get<is_region_state>(state).add_r1_external(m_layer_data);

    for_each_i<qt_num>([&]<unsigned QT> {
      dlm::init_maps(m_q_data, m_layer_data, dlm::ckey<QT, 0>{}, dlm::ckey<0, 0, QT, 0>{});
      get<is_region_state>(state).template add_r2_external<QT>(m_q_data, dlm::ckey<QT>{});
    });
  }

  god::state<Ts...> *m_state;
};

struct layer_state {
  template<typename... Ts>
  using template_t = layer_state_tt<Ts...>;
};
} // namespace cp6

/*
  Should the nested layer system be in here?
    We could have it be in another state, but having it in here
    would save us a permute map and would centralize the whole system.
    There may be situations where we don't need the sublayering, like for
    normal games. Eh, if you're making a normal game, then you probably don't
    care as much about storage or performance overhead that comes with the
    nested layers. I'll do it in another system for now and I'll merge them
    if need be.


  How are sub-layer sets going to collide?
    if a and b share a common {ls} ancestor:
      if they belong to the same {lay} in that ancestor:
        they can collide
      else:
        the cannot collide
    else:
      they can collide

    How are we gonna find common ancestors?
      Some sort of recursive algorithim or heap alloctation.



  If a and b are part of the same layer set but are on different layers, the cannot
  collide.


  The the name global_layer_system doesn't make sense. It doesn't globally layer objects,
  instead it describes what sets of objects can and cannot collide. So, it should be
  called phase_set_system. This allows us to just use layer_system for this, and sub_layer_system
  for the addition.

  This layer system ill equipt to handle nested layer sets.
    If you have a box in another box, then the inner box would
    create 3 more middle layers, or it would collide with itself.

    We could add this functionality in by adding another system that gives each
    layer set the abiltiy to reside within another layer set.

      Some rules for this system:
        Each layer set can only belong to one layer of another layer set.
        Each layer set can have any amount of children layers.
        When a layer set is inside another, it's qs and sub layers cannot collide
        with the q's in the layers above or below it.

      This system is basically the old front/below set except on the layer set
      instead of q set and without calculating the local layers from it.

      With this addition, we could build a bunch of boxes and put them in one large box.

      When an object is removed from it's layer set, it needs to be put in the parent
      layer its old layer set was in.

      Are there any obvious problems with this approach? Well it's purly additive
      to the old layer system, so it doesn't change it's functionality. Because
      we aren't calculating local layers like in the q based approach, we shouldn't
      run into any of the problems associated with that.



  How should I structure the index sets for the local_layer_system.
    Well we need to align state with each <qt>[q].
    Layer sets are likely to persist for a while, with qs coming and going
    reguarly (water leaking out of a pot).

    Because of this we should probably have the layer set like:
      [reg][lls][llay]<qt>[llay_q] <=> [reg]<qt>[q]
    with each <qt> having a reserved space.

  I should rename some things.
    order_system -> local_layer_system
    layer_system -> global_layer_system

    I'm going to add:
      union_system


  What happens when a q is removed?
    All of it's children become roots. Imagine a box with a front middle and back, and in the
    middle layer there is a bunch of little balls. When the front(root) gets removed all of
    the little balls become roots and collide with the back wall, flinging them everywhere.

    There is more to this.

    Let's consider only the [root] set, the back/behind set is only one way to build the root set.
    Lets also ignore using layers for unionizing. In this case, there is no root, just layers of
  data. So, if a body gets removed all other bodies are unaffected. This is the base functionality
  we need from this system. How we build/remove these layers is irrelevant to this system.

      So, how will we build layers? A manual approach depending on what we are trying to do is
  probably best.

      How will unionizing qs work? Well, since we need a root for this, and this system no longer
      has roots, another system would handle this.

      What is the connection between unionized q and layers?
        There is no connection. They are mutually exclusive

      With these two systems we can build a box by unionizing the sides front and back together, and
  put them all under the same layer set, with the front being in the first layer, the sides in the
  second, and the back in the third. We could then put balls in this box by putting them in the
  second layer. Now when the front is removed, the second layer becomes the first, and the box stays
  intact (or it could break a part if we want).

      Are there any cases where this doesn't work? Well, the jobs of each system are fairly well
  defined: Unionized qs determine to q that constraints are added to and stops unionized qs from
        colliding with each other.

        Local layers only determine what qs cannot collide with.
      And their shared no-collide functionality do not interact or have any unexpected side effects
  with each other. So no, there shouldn't be any cases where this doesn't work. But I'll try to
  think of some anyway.

        It is going to be challenging to make a broad phase that efficiently takes both into
  account. Really, the fastest way could be to cull potential collisions on the gpu after simple
  broadphase.





  lay is already taken by the layering system. We could just use llay for local layer

  We don't really need the root/behind map for anything. What we need is the [root][lay] map:
    [reg][root][lay][qt][lay_q] <=> [qt][reg][q]
  The root/behind map is only used for building this map. So, how do we dynamically maintain
  both maps?
    We could just store both and when the behind/front one gets updated, we update the layer
    one. This is kinda what we are already doing.
    Is there any other ways of doing this? Not that I can think of.

  NVM:
    We should have the order under the root node be able to collide with the root node.
    Having it inclusive like this will just be better.

    Having the root be inclusive doesn't really solve any problems, it just makes things
    more complecated.

  Should we have the roots be the front most or back most objects?

  I want liquid to be able to be able to leak out of objects through
  cracks. If the roots are the back most object, liquid can only leak
  through cracks in the front which is better than having it only leak
  through cracks in the back, but preferably we want it to be able to
  leak out both sides.

  if we had a infront/behind the root flag

  Do we even want liquid to go in front of a root object? We can have leaking
  how we want if the root object can collide.
*/
