#pragma once
#include "avbd/avbd_flags.h"
#include "avbd/avbd_system.h"
#include "avbd/collision/avbd_pcnt_fg.h"
#include "avbd/collision/avbd_tcnt_fg.h"
#include "avbd/color_system.h"
#include "avbd_basic/liquid/avbd_liquid_fg.h"
#include "avbd_basic/vbd_3d_qd.h"
#include "collision/narrowphase_system.h"
#include "collision/pcnt_system.h"
#include "collision_basic/basic_broadphase.h"
#include "avbd_basic/constraint/avbd_pin_fg.h"
#include "god_state.h"
#include "region_state.h"
#include "state.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/View.hpp>
#include <consistant_caller.h>
// needed for state definition
#include <avbd_basic/collision/avbd_cnt_type_defs.h>
#include <collision_basic/collision_combo_defs.h>



struct vbd_liquid_qt:
dlm::fdst<cp6::vbd_liquid_qd, cp6::circle_cd>,
  cp6::flag_vbd_qt,
  cp6::flag_avbd_tcnt_force {};

DISABLE_COLLISION_FORCE(vbd_liquid_qt, vbd_liquid_qt)

struct vbd_particle_qt:
dlm::fdst<cp6::vbd_2d_qd, cp6::circle_cd>,
  cp6::flag_vbd_qt,
  cp6::flag_avbd_tcnt_force {};

struct vbd_poly_qt:
dlm::fdst<cp6::vbd_3d_qd, cp6::poly_cd>,
  cp6::flag_vbd_qt,
  cp6::flag_plqt,
  cp6::flag_pcqt,
  cp6::flag_avbd_pinnable,
  cp6::flag_avbd_pcnt,
  cp6::flag_gs_for_avbd_pcnt,
  cp6::flag_colorable {
  static constexpr bool use_impulses = true;
};

struct avbd_test_state: state {

  avbd_test_state(state_man &man);
  void update();
  void draw(sf::RenderWindow &window);

  enum qt_ids {
    liquid_qt,
    particle_qt,
    poly_qt,
    qt_num
  };
  // There's only one region in this demo
  enum reg_id { reg, reg_num };

  using reg_t = cp6::region_state<
    vbd_liquid_qt,
    vbd_particle_qt,
    vbd_poly_qt
  >;

  // type wrapper
  struct tw {
    using types = std::tuple<
      reg_t,
      // collision
      cp6::basic_broadphase,
      cp6::narrowphase_system,
      cp6::phase_link_system,
      cp6::pcnt_system,
      cp6::pcol_state,
      // avbd
      cp6::color_system,
      cp6::avbd_system,
      cp6::avbd_pcnt_fg,
      cp6::avbd_tcnt_fg,
      cp6::avbd_pin_fg,
      cp6::avbd_liquid_fg
    >;
  };

  struct physic_state: god::state<tw> {
  } m_phy;

  consistant_caller caller;

  float liquid_particle_radius = 10;
  float liquid_kernal_collider_radius = 30;


  static constexpr unsigned color_color_num = 8;
  static constexpr sf::Color color_colors[] = {
    {255, 0, 0, 200},
    {0, 255, 0, 200},
    {0, 0, 255, 200},
    {255, 0, 255, 200},
    {255, 255, 0, 200},
    {0, 255, 255, 200},
    {125, 125, 0, 200},
    {125, 0, 125, 200}
  };

  template<unsigned QT>
  auto &get_qt() {
    return get<cp6::is_region_state>(m_phy).template get_qt<QT>(reg);
  }

  unsigned add_poly(vec2f pos, vector_poly shape);
  unsigned add_particle(vec2f pos, float radius);
  unsigned add_liquid_particle(vec2f pos);
};
