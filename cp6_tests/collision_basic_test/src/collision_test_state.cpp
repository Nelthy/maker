#include "collision/pcol_state.h"
#include "collision_basic/narrowphase_fns.h"
#include "collision_basic/narrowphase_update_fns.h"
#include "collision_basic/overlapping_defs.h"
#include "collision_basic/pcnt_fns.h"
#include "collision_basic/pcol_fns.h"
#include "collision_basic/transform_point_defs.h"

#include "collision_basic_draw/draw_cd.h"
#include "collision_basic_draw/draw_rb_col_state.h"
#include "collision_basic_draw/to_sf.h"

#include "collision_test_state.h"
#include "print_functions.h"
#include "random_index.h"
#include <iostream>
template<unsigned QT>
using qt_t = std::decay_t<decltype(std::declval<collision_test_state>().get_qt<QT>())>;

#define LOG_LINE std::cout << "LINE: " << __LINE__ << std::endl;
void on_main_spin(){};

template<typename QT1, typename QT2>
void resolve_collisions(const cp6::contact_1p_set &cnts, QT1 &qt1, QT2 &qt2) {
  for (unsigned i = 0; i < cnts.size(); i++) {
    const cp6::contact_1p &cnt = cnts[i];
    vec2f trans1 = qt1.get_trans(cnt.q1);
    vec2f trans2 = qt2.get_trans(cnt.q2);
    float d = dot(cnt.n, cnt.x2 - cnt.x1);
    qt1.set_trans(cnt.q1, trans1 + cnt.n * d * 0.5);
    qt2.set_trans(cnt.q2, trans2 - cnt.n * d * 0.5);
  }
}

template<typename QT1, typename QT2>
void resolve_collisions(const cp6::contact_2p_set &cnts, QT1 &qt1, QT2 &qt2) {
  for (unsigned i = 0; i < cnts.size(); i++) {
    const cp6::contact_2p &cnt = cnts[i];
    vec2f trans1 = qt1.get_trans(cnt.q1);
    vec2f trans2 = qt2.get_trans(cnt.q2);
    float d = dot(cnt.cnt1.n, cnt.cnt1.x2 - cnt.cnt1.x1);
    qt1.set_trans(cnt.q1, trans1 + cnt.cnt1.n * d * 0.5);
    qt2.set_trans(cnt.q2, trans2 - cnt.cnt1.n * d * 0.5);
  }
}

unsigned collision_test_state::add_circle(vec2f pos) {
  unsigned q = get_qt<circle_qt>().size();
  grow<circle_qt>(get<cp6::is_region_state>(m_phy), 1, {reg});
  get_qt<circle_qt>().td()[q] = pos;

  return q;
}

unsigned collision_test_state::add_poly(vec2f pos, vector_poly shape) {
  unsigned q = get_qt<poly_qt>().size();
  grow<poly_qt>(get<cp6::is_region_state>(m_phy), 1, {reg});
  get_qt<poly_qt>().td()[q].xy() = pos;
  get_qt<poly_qt>().cd().base_cd()[q] = shape;

  return q;
}

unsigned collision_test_state::add_root(vec2f pos) {
  unsigned rq = get_qt<root_qt>().size();
  grow<root_qt>(get<cp6::is_region_state>(m_phy), 1, {reg});

  get_qt<root_qt>().td()[rq].xy() = pos;
  get_qt<root_qt>().cd()[rq].radius = 5;
  return rq;
}

unsigned collision_test_state::add_circle_child(
  unsigned rq,
  vec2f body_translation,
  float body_rotation
) {
  auto &union_s = get<cp6::union_system<test_root_qt>>(m_phy);
  unsigned cq = get_qt<circle_cqt>().size();
  dlm::grow<circle_cqt>(get<cp6::is_region_state>(m_phy), 1, {reg});

  union_s.link_to_root(reg, rq, circle_cqt, cq);
  union_s.body_translation(reg, circle_cqt)[cq] = body_translation;
  union_s.body_rotation(reg, circle_cqt)[cq] = body_rotation;
  return cq;
}

unsigned collision_test_state::add_poly_child(
  unsigned rq,
  vector_poly shape,
  vec2f body_translation,
  float body_rotation
) {
  auto &union_s = get<cp6::union_system<test_root_qt>>(m_phy);
  unsigned cq = get_qt<poly_cqt>().size();
  dlm::grow<poly_cqt>(get<cp6::is_region_state>(m_phy), 1, {reg});

  get_qt<poly_cqt>().cd().base_cd()[cq] = shape;
  union_s.link_to_root(reg, rq, poly_cqt, cq);
  union_s.body_translation(reg, poly_cqt)[cq] = body_translation;
  union_s.body_rotation(reg, poly_cqt)[cq] = body_rotation;
  return cq;
}

unsigned collision_test_state::add_basic_circle(vec2f pos) {
  unsigned q = get_qt<basic_qt>().size();
  dlm::grow<basic_qt>(get<cp6::is_region_state>(m_phy), 1, {reg});
  get_qt<basic_qt>().set_trans(q, pos);
  return q;
}

collision_test_state::collision_test_state(state_man &man) : state(man) {
  std::cout << "Starting collision_test_state" << std::endl;

  get<cp6::is_region_state>(m_phy).add_r1_external(m_debug_reg_q);
  for_each_i<qt_num>([&]<size_t QT> {
    get<cp6::is_region_state>(m_phy).add_r2_external<QT>(m_debug_reg_q, dlm::ckey<QT>{});
  });

  dlm::grow(get<cp6::is_region_state>(m_phy), reg_num);
  get_qt<poly_cqt>().m_root_qt = &get_qt<root_qt>();
  get_qt<circle_cqt>().m_root_qt = &get_qt<root_qt>();
  get_qt<poly_cqt>().to_rq = &get<cp6::union_system<test_root_qt>>(m_phy).to_rq(reg, poly_cqt);
  get_qt<circle_cqt>().to_rq =
    &get<cp6::union_system<test_root_qt>>(m_phy).to_rq(reg, circle_cqt);

  add_circle({100, 100});
  add_poly({100, 200}, vector_poly::hexagon_shape());
  // add_poly({200, 200}, vector_poly::hexagon_shape());
  // add_circle({200, 200});
  add_poly({200, 100}, vector_poly::square_shape());

  // unsigned rq = add_root({300, 300});
  //
  // {
  //   add_circle_child(rq, {100, 0});
  //   add_circle_child(rq, {-100, 0});
  //   add_poly_child(rq, vector_poly::square_shape(), {0, 100});
  //   add_poly_child(rq, vector_poly::square_shape(), {0, -100});
  // }

  // // making a bucket
  // {
  //   rq = add_root({300, 50});
  //   // create a layer state
  //   unsigned ls = size<0>(get<cp6::layer_state>(m_phy).m_layer_data, {reg});
  //   grow<0>(get<cp6::layer_state>(m_phy).m_layer_data, 1, {reg});
  //   // add two layers in that layer state
  //   grow<0, 0>(get<cp6::layer_state>(m_phy).m_layer_data, 2, {reg, ls});
  //   // add rq to the back layer ([1])
  //   get<cp6::layer_state>(m_phy).m_layer_data.to_q<root_qt>().sublink(
  //     dlm::ikey{reg, ls, 1},
  //     dlm::ikey{reg, rq}
  //   );
  //   {
  //     // add the back, setting to the back layer [1]
  //     unsigned cq = add_poly_child(rq, vector_poly::square_shape(3, 3), {0, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 1},
  //       dlm::ikey{reg, cq}
  //     );
  //
  //     // add the sides, setting each to the front layer [0]
  //     cq = add_poly_child(rq, vector_poly::square_shape(3, .2), {0, 75});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //     cq = add_poly_child(rq, vector_poly::square_shape(.2, 3), {75, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //     cq = add_poly_child(rq, vector_poly::square_shape(.2, 3), {-75, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //   }
  //   // add ball to put in bucket
  //   unsigned q = add_circle({300, 10});
  //   get<cp6::layer_state>(m_phy).m_layer_data.to_q<circle_qt>().sublink(
  //     dlm::ikey{reg, ls, 0},
  //     dlm::ikey{reg, q}
  //   );
  // }
  //
  // // making a smaller bucket
  // {
  //   rq = add_root({200, 50});
  //
  //   // create a layer state
  //   unsigned ls = size<0>(get<cp6::layer_state>(m_phy).m_layer_data, {reg});
  //   grow<0>(get<cp6::layer_state>(m_phy).m_layer_data, 1, {reg});
  //   // add two layers in that layer state
  //   grow<0, 0>(get<cp6::layer_state>(m_phy).m_layer_data, 2, {reg, ls});
  //   // add rq to the back layer ([1])
  //   get<cp6::layer_state>(m_phy).m_layer_data.to_q<root_qt>().sublink(
  //     dlm::ikey{reg, ls, 1},
  //     dlm::ikey{reg, rq}
  //   );
  //   {
  //     // add the back, setting to the back layer [1]
  //     unsigned cq = add_poly_child(rq, vector_poly::square_shape(3.f / 2.f, 3.f / 2.f), {0, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 1},
  //       dlm::ikey{reg, cq}
  //     );
  //
  //     // add the sides, setting each to the front layer [0]
  //     cq = add_poly_child(rq, vector_poly::square_shape(3.f / 2.f, .2f / 2.f), {0, 75.f / 2.f});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //     cq = add_poly_child(rq, vector_poly::square_shape(.2f / 2.f, 3.f / 2.f), {75.f / 2.f, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //     cq = add_poly_child(rq, vector_poly::square_shape(.2f / 2.f, 3.f / 2.f), {-75.f / 2.f, 0});
  //     get<cp6::layer_state>(m_phy).m_layer_data.to_q<poly_cqt>().sublink(
  //       dlm::ikey{reg, ls, 0},
  //       dlm::ikey{reg, cq}
  //     );
  //   }
  //   // add ball to put in bucket
  //   unsigned q = add_circle({200, 10});
  //   get_qt<circle_qt>().cd()[q].radius /= 2;
  //   get<cp6::layer_state>(m_phy).m_layer_data.to_q<circle_qt>().sublink(
  //     dlm::ikey{reg, ls, 0},
  //     dlm::ikey{reg, q}
  //   );
  //
  //   // putting this smaller bucket in a sublayer under the bigger one
  //   get<cp6::sub_layer_system>(m_phy).add_sublayer(reg, {0, 0}, {1});
  // }

  if (!load_keys_from_config("basic_keys.ini")) {
    assert(false);
  };
}

void collision_test_state::update() {
  // quit
  if (keybinds["Q"]->ispressed) {
    this->background_state = Remove;
  }

  // reset
  if (keybinds["R"]->ispressed) {
    this->background_state = Remove;
    m_man.push_new_state<collision_test_state>();
  }

  // swap
  if (keybinds["S"]->isheld) {
    for_each_i<qt_num>([&]<size_t QT> {
      if (size<QT>(get<cp6::is_region_state>(m_phy), reg) > 0) {
        for (unsigned i = 0; i < size<QT>(get<cp6::is_region_state>(m_phy), reg); i++) {
          swap<QT>(
            get<cp6::is_region_state>(m_phy),
            {reg, random_index(0, get_qt<QT>().size() - 1)},
            {reg, random_index(0, get_qt<QT>().size() - 1)}
          );
        }
      }
    });
  }

  // delete
  if (keybinds["D"]->ispressed || keybinds["D"]->timeheld > 0.1) {
    auto &reg_s = get<cp6::is_region_state>(m_phy);
    if (size<circle_qt>(reg_s, reg) > 0) {
      null<circle_qt>(reg_s, {reg, 0});
    } else if (size<poly_qt>(reg_s, reg) > 0) {
      null<poly_qt>(reg_s, {reg, 0});
    } else if (size<basic_qt>(reg_s, reg) > 0) {
      null<basic_qt>(reg_s, {reg, 0});
    } else if (size<poly_cqt>(reg_s, reg) > 0) {
      null<poly_cqt>(reg_s, {reg, 0});
    } else if (size<circle_cqt>(reg_s, reg) > 0) {
      null<circle_cqt>(reg_s, {reg, 0});
    } else if (size<root_qt>(reg_s, reg) > 0) {
      null<root_qt>(reg_s, {reg, 0});
    }
    clear_null(reg_s);
  }

  get<cp6::union_system<test_root_qt>>(m_phy).to_rq(reg, circle_cqt);
  // transforming collider data
  {
    for_each_i<qt_num>([&]<size_t QT> {
      cd_transform_start(get_qt<QT>().cd());

      if constexpr (cp6::is_union_child<test_root_qt>::pred<qt_t<QT>>::value) {
        auto &root = get<cp6::is_region_state>(m_phy).get_qt<root_qt>(reg);
        auto &union_s = get<cp6::union_system<test_root_qt>>(m_phy);
        cd_transform(get_qt<QT>().cd(), union_s.body_rotation(reg, QT));
        cd_transform(get_qt<QT>().cd(), union_s.body_translation(reg, QT));
        cd_ind_transform(
          get_qt<QT>().cd(),
          root.td(),
          (unsigned *)get<cp6::union_system<test_root_qt>>(m_phy).to_rq(reg, QT).data()
        );
      } else {
        cd_transform(get_qt<QT>().cd(), get_qt<QT>().td());
      }

      cd_transform_end(get_qt<QT>().cd());
    });
  }

  get<cp6::basic_broadphase>(m_phy).update(reg);
  get<cp6::phase_link_system>(m_phy).update(reg);
  // culling potential collisions
  {
    // // union child-child culling
    // tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
    //   if constexpr (cp6::is_union_child<test_root_qt>::pred<qt_t<QT1>>::value &&
    //                 cp6::is_union_child<test_root_qt>::pred<qt_t<QT2>>::value) {
    //     std::vector<ab_link> &potcols =
    //       get<cp6::basic_broadphase>(m_phy).get_potcols<QT1, QT2>(reg);
    //     for (unsigned i = 0; i < potcols.size(); i++) {
    //       if (!get<cp6::union_system<test_root_qt>>(m_phy)
    //              .template can_collide<QT1, QT2>(reg, potcols[i].a, potcols[i].b)) {
    //         potcols[i] = potcols.back();
    //         potcols.pop_back();
    //         i--;
    //       }
    //     }
    //   }
    // });
    //
    // // union root-child culling
    // tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
    //   if constexpr (cp6::is_union_child<test_root_qt>::pred<qt_t<QT2>>::value) {
    //     using root_t = typename qt_t<QT2>::root_qt;
    //     if constexpr (std::is_same_v<qt_t<QT1>, root_t>) {
    //       std::vector<ab_link> &potcols =
    //         get<cp6::basic_broadphase>(m_phy).get_potcols<QT1, QT2>(reg);
    //       auto &union_s = get<cp6::union_system<test_root_qt>>(m_phy);
    //       for (unsigned i = 0; i < potcols.size(); i++) {
    //         unsigned rq = union_s.to_rq(reg, QT2)[potcols[i].b];
    //
    //         if (potcols[i].a == rq) {
    //           potcols[i] = potcols.back();
    //           potcols.pop_back();
    //           i--;
    //         }
    //       }
    //     }
    //   }
    // });

    // // layer culling
    // tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
    //   std::vector<ab_link> &potcols =
    //     get<cp6::basic_broadphase>(m_phy).get_potcols<QT1, QT2>(reg);
    //   for (unsigned i = 0; i < potcols.size(); i++) {
    //     if (!get<cp6::layer_state>(m_phy)
    //            .template can_collide<QT1, QT2>(reg, potcols[i].a, potcols[i].b)) {
    //       potcols[i] = potcols.back();
    //       potcols.pop_back();
    //       i--;
    //     }
    //   }
    // });
    //
    // // sub-layer culling
    // tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
    //   std::vector<ab_link> &potcols =
    //     get<cp6::basic_broadphase>(m_phy).get_potcols<QT1, QT2>(reg);
    //   for (unsigned i = 0; i < potcols.size(); i++) {
    //     if (!get<cp6::sub_layer_system>(m_phy)
    //            .template can_collide<QT1, QT2>(reg, potcols[i].a, potcols[i].b)) {
    //       potcols[i] = potcols.back();
    //       potcols.pop_back();
    //       i--;
    //     }
    //   }
    // });
  }
  get<cp6::narrowphase_system>(m_phy).update(reg);
  get<cp6::pcnt_system>(m_phy).update();

  // auto& pcnt_s = get<cp6::persistent_contact_system>(m_phy);
  // std::cout << pcnt_s.get_pcnt_data<circle_qt, poly_qt>(reg).size() << std::endl;
  // resolving collisions
  {
    // persistent contacts
    if (!keybinds["T"]->isheld) {

      tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
        if constexpr (cp6::is_pcqt<qt_t<QT1>>::value && cp6::is_pcqt<qt_t<QT2>>::value) {
          resolve_collisions(
            get<cp6::pcnt_system>(m_phy).get_pcnt_data<QT1, QT2>(reg),
            get_qt<QT1>(),
            get_qt<QT2>()
          );
        }
      });
      // temporary contacts
      tri_apply<qt_num>([&]<unsigned QT1, unsigned QT2> {
        if constexpr ((!cp6::is_pcqt<qt_t<QT1>>::value || !cp6::is_pcqt<qt_t<QT2>>::value)) {
          resolve_collisions(
            get<cp6::narrowphase_system>(m_phy).get_tcnt_data<QT1, QT2>(reg),
            get_qt<QT1>(),
            get_qt<QT2>()
          );
        }
      });
    }
  }
}

void collision_test_state::draw(sf::RenderWindow &window) {
  static constexpr unsigned mouse_qt = circle_qt;
  dlm::ikey<2> mouse_q = {mouse_qt, 0};
  vec2f mouse_pos = from_sf(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
  if (keybinds["LShift"]->isheld) {
    if (get_qt<mouse_qt>().size() > 0) {
      get_qt<mouse_qt>().set_trans(0, mouse_pos);
    } else {
      add_circle(mouse_pos);
    }
  }
  for_each_i<qt_num>([&]<size_t QT> {
    draw_cd(window, get_qt<QT>().cd(), sf::Color{103, 109, 123, 200});
  });

  // drawing persistant contacts green
  tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
    if constexpr (cp6::is_pcqt<qt_t<QT1>>::value && cp6::is_pcqt<qt_t<QT2>>::value) {

      draw_col_state(
        window,
        get<cp6::pcnt_system>(m_phy).template get_pcnt_data<QT1, QT2>(reg),
        {0, 0},
        sf::Color::Green,
        sf::Color::Green
      );
    }
  });

  // drawing contacts red and blue
  tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
    draw_col_state(
      window,
      get<cp6::narrowphase_system>(m_phy).template get_tcnt_data<QT1, QT2>(reg),
      {0, 0}
    );
  });

  // add
  if (keybinds["A"]->ispressed || keybinds["A"]->timeheld > 0.1) {
    unsigned qt = random_index(0, qt_num - 1);
    if (qt == circle_qt) {
      unsigned q = add_circle(mouse_pos);
      get_qt<circle_qt>().cd()[q].radius = random_index(10, 60);
    } else if (qt == poly_qt) {
      unsigned q = add_poly(mouse_pos, vector_poly::square_shape());
      get_qt<poly_qt>().td()[q].z = mouse_pos.x;
    } else if (qt == basic_qt) {
      unsigned q = add_basic_circle(mouse_pos);
      get_qt<basic_qt>().cd()[q].radius = 10;
    }
  }
}
