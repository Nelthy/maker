#pragma once
#include "collision/collision_combo_decl.h"
#include "collision/collision_flags.h"
#include "collision/layer_state.h"
#include "collision/narrowphase_system.h"
#include "collision/pcnt_system.h"
#include "collision/pcol_state.h"
#include "collision/phase_link_system.h"
#include "collision/sub_layer_system.h"
#include "collision/union_system.h"
#include "collision_basic/basic_broadphase.h"
#include "collision_basic/circle_cd.h"
#include "collision_basic/poly_cd.h"
#include "flat_data_set_tuple.h"
#include "god_state.h"
#include "print_ds.h"
#include "region_state.h"
#include "size_ds.h"
#include "state.h"
#include "vec_ds.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <collision_basic/collision_combo_defs.h>

struct collision_test_state;

struct transform_data_2d: dlm::dst<dlm::vec_ds<vec2f>> {
  vec2f *td() { return dlm::get<0>(*this).data(); }

  const vec2f *td() const { return dlm::get<0>(*this).data(); }

  void set_trans(unsigned q, vec2f trans) { td()[q] = trans; }

  vec2f get_trans(unsigned q) { return td()[q]; }
};

struct transform_data_3d: dlm::dst<dlm::vec_ds<vec3f>> {

  vec3f *td() { return dlm::get<0>(*this).data(); }

  const vec3f *td() const { return dlm::get<0>(*this).data(); }

  void set_trans(unsigned q, vec2f trans) { td()[q].xy() = trans; }

  vec2f get_trans(unsigned q) { return td()[q].xy(); }
};

struct test_circle_qt:
dlm::fdst<cp6::circle_cd, transform_data_2d>,
  cp6::flag_plqt,
  cp6::flag_pcqt {};

struct test_basic_circle_qt: dlm::fdst<cp6::circle_cd, transform_data_2d> {};

struct test_poly_qt: dlm::fdst<cp6::poly_cd, transform_data_3d>, cp6::flag_plqt, cp6::flag_pcqt {};

struct test_root_qt: dlm::fdst<cp6::circle_cd, transform_data_3d> {};

struct test_poly_cqt: dlm::fdst<cp6::poly_cd, transform_data_3d>, cp6::flag_plqt, cp6::flag_pcqt {
  using root_qt = test_root_qt;

  void set_trans(unsigned q, vec2f trans) { m_root_qt->td()[to_rq->operator[](q)].xy() = trans; }

  vec2f get_trans(unsigned q) { return m_root_qt->td()[to_rq->operator[](q)].xy(); }

  root_qt *m_root_qt{};
  const dlm::vec_ds<dlm::ikey<1>> *to_rq{};
};

struct test_circle_cqt: dlm::fdst<cp6::circle_cd, transform_data_2d> {
  using root_qt = test_root_qt;

  void set_trans(unsigned q, vec2f trans) { m_root_qt->td()[to_rq->operator[](q)].xy() = trans; }

  vec2f get_trans(unsigned q) { return m_root_qt->td()[to_rq->operator[](q)].xy(); }

  root_qt *m_root_qt{};
  const dlm::vec_ds<dlm::ikey<1>> *to_rq{};
};

struct basic_color_data {
  const unsigned *to_q;
  unsigned cq_num;
};

DISABLE_QT_COLLISIONS(test_basic_circle_qt, test_basic_circle_qt)

struct collision_test_state: state {

  collision_test_state(state_man &man);
  void update();
  void draw(sf::RenderWindow &window);

  enum reg_id { reg, reg_num };

  enum qts { root_qt, circle_cqt, circle_qt, basic_qt, poly_cqt, poly_qt, qt_num };

  dlm::ext<dlm::perm<
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "root qt">>>>,
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "circle cqt">>>>,
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "circle qt">>>>,
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "basic qt">>>>,
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "poly cqt">>>>,
    dlm::ext<dlm::perm<dlm::dst<dlm::vec_ds<unsigned, 2>, dlm::print_ds<2, "poly qt">>>>,
    dlm::dst<dlm::size_ds, dlm::print_ds<1, "reg">>
  >>
    m_debug_reg_q;

  using reg_t = cp6::region_state<
    test_root_qt,
    test_circle_cqt,
    test_circle_qt,
    test_basic_circle_qt,
    test_poly_cqt,
    test_poly_qt
  >;

  // type wrapper
  struct tw {
    using types = std::tuple<
      reg_t,
      cp6::basic_broadphase,
      cp6::narrowphase_system,
      cp6::phase_link_system,
      cp6::pcnt_system,
      cp6::layer_state,
      cp6::sub_layer_system,
      cp6::union_system<test_root_qt>,
      cp6::pcol_state
    >;
  };

  god::state<tw> m_phy;

  static_assert(cp6::is_region_state<reg_t>::value);
  static constexpr auto f_to_u = filter_tuple<cp6::is_region_state, full_tuple_t<tw>>();
  static_assert(f_to_u.size() == 1);
  static_assert(f_to_u[0] == 0);

  template<unsigned QT>
  inline auto &get_qt() {
    auto &ref = get<reg_t>(m_phy);
    return ref.template get_qt<QT>(0);
  };

  template<typename QT>
  inline auto &get_qt() {
    auto &ref = get<reg_t>(m_phy);
    return ref.template get_qt<QT>(0);
  };

  static constexpr sf::Color m_color_colors[7] = {
    {255, 0, 0},
    {0, 255, 0},
    {0, 0, 255},
    {125, 125, 0},
    {0, 125, 125},
    {125, 0, 125},
    {125, 125, 125}
  };

  unsigned add_circle(vec2f pos);
  unsigned add_poly(vec2f pos, vector_poly shape);
  unsigned add_root(vec2f pos);
  unsigned add_circle_child(unsigned rq, vec2f body_translation, float body_rotation = 0);
  unsigned add_poly_child(
    unsigned rq,
    vector_poly shape,
    vec2f body_translation,
    float body_rotation = 0
  );
  unsigned add_basic_circle(vec2f pos);
};
