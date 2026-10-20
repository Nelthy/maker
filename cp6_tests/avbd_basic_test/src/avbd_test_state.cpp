// needed for definitions
#include "avbd_basic/collision/avbd_cnt_fn_defs.h"
#include "avbd_basic/constraint/avbd_pin_fg.h"
#include "avbd_basic/vbd_2d_qd.h"
#include "collision_basic/narrowphase_fns.h"
#include "collision_basic/narrowphase_update_fns.h"
#include "collision_basic/overlapping_defs.h"
#include "collision_basic/pcnt_fns.h"
#include "collision_basic/pcol_fns.h"
#include "collision_basic/transform_point_defs.h"
#include "collision_basic_draw/draw_cd.h"
#include "collision_basic_draw/draw_rb_col_state.h"
#include "collision_basic_draw/to_sf.h"

#include "PA.hpp"
#include "avbd_test_state.h"
#include "flags.h"
#include "random_index.h"
#include "scalers.h"

#include "avbd_basic/constraint/avbd_pin_impl.h"
#include "tri_utils.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstddef>
#include <utility>

void on_main_spin() {};

template<unsigned QT>
using qt_t = std::decay_t<decltype(std::declval<avbd_test_state>()
                                     .get_qt<QT>())>;

avbd_test_state::avbd_test_state(state_man &man) : state(man) {
  reg_t &reg_s = get<reg_t>(m_phy);

  dlm::grow(reg_s, 1);

  // adding mouse collider
  {
    float mouse_scale = .01;
    unsigned q = add_poly(
      from_sf(m_view.getCenter()) + vec2f{0, 0},
      vector_poly::square_shape(mouse_scale, mouse_scale)
    );
    get_qt<poly_qt>().flags()[q].immovable = true;
    get_qt<poly_qt>().flags()[q].dont_resolve_collisions = true;
  }

  // adding walls
  {
    // bottom
    unsigned q = add_poly(
      from_sf(m_view.getCenter()) + vec2f{1000, 700},
      vector_poly::square_shape(10000, 20)
    );
    get_qt<poly_qt>().flags()[q].immovable = true;

    // left
    q = add_poly(
      from_sf(m_view.getCenter()) + vec2f{-5000},
      vector_poly::square_shape(20, 10000)
    );
    get_qt<poly_qt>().flags()[q].immovable = true;

    // right
    q = add_poly(
      from_sf(m_view.getCenter()) + vec2f{5000, 0},
      vector_poly::square_shape(20, 10000)
    );
    get_qt<poly_qt>().flags()[q].immovable = true;

  }

  // creating stack
  vec2f bottom_pos{0, 450};
  for (unsigned i = 0; i < scalers::get("stack height", 00, 20); i++) {
    add_poly(
      bottom_pos - vec2f{0, 50 * 0.25f * i},
      vector_poly::square_shape(10 + (0.01 * float(i)), 0.25)
    );
  }

  // creating pyramid
  {
    // 80
    unsigned base_size = scalers::get("pyramid base size", 10, 1);
    float box_size_x = 51;
    float box_size_y = 49;
    float base_bottom_y = 450;
    float base_center_x = 500;

    for (unsigned r = 0; r < base_size; r++) {
      unsigned boxes_in_row = base_size - r;
      float row_y = base_bottom_y - (r + 0.5) * box_size_y;
      float row_total_width = boxes_in_row * box_size_x;
      float row_start = base_center_x - row_total_width / 2.f
                      + box_size_x / 2.f;
      for (unsigned i = 0; i < boxes_in_row; i++) {
        if (r % 2 == 0 || i % 2 == 0 || !flags::get("sparse pyramid", false)) {
          float box_x = row_start + i * box_size_x;
          add_poly({box_x, row_y}, vector_poly::square_shape());
        }
      }
    }
  }

  // add_poly({358, 293}, vector_poly::square_shape(2, 2));
  // add_poly({289, 395}, vector_poly::square_shape(2, 2));

  // creating chain
  {
    unsigned chain_length = scalers::get("chain length", 10, 1);
    vec2f chain_start = from_sf(m_view.getCenter())
                      + vec2f{-400, -float(chain_length * 50)};
    for (unsigned i = 0; i < chain_length; i++) {
      if (i > 0) {
        cp6::pin_data pin;
        if (i % 2 == 1) {
          pin.ra = {25, 25};
          pin.rb = {25, -25};
        } else {
          pin.ra = {-25, 25};
          pin.rb = {-25, -25};
        }

        unsigned q = add_poly(
          chain_start + vec2f{0, float(i * 51)},
          vector_poly::square_shape()
        );
        get<cp6::avbd_pin_fg>(m_phy)
          .add_pin<poly_qt, poly_qt>(reg, q - 1, q, pin);
      } else {
        // first link
        unsigned q = add_poly(
          chain_start + vec2f{0, float(i * 51)},
          vector_poly::square_shape()
        );
        // get_qt<poly_qt>().flags()[q].dont_resolve_collisions = true;
        // get_qt<poly_qt>().flags()[q].immovable = true;
      }
    }
  }

  // creating liquid
  // {
  //   float dist = 25;
  //   const unsigned liquid_depth = scalers::get("liquid depth", 0, 5);
  //   const float x_bound = m_view.getCenter().x + m_view.getSize().x / 2;
  //
  //   vec2f curr_pos =
  //     from_sf(m_view.getCenter()) - vec2f{m_view.getSize().x / 2,
  //     -m_view.getSize().y / 2};
  //   unsigned curr_depth = 0;
  //   while (curr_depth < liquid_depth + 1) {
  //     add_liquid_particle(curr_pos);
  //     curr_pos.x += dist;
  //     if (x_bound - 60 < curr_pos.x) {
  //       curr_pos.x = m_view.getCenter().x - m_view.getSize().x / 2;
  //       curr_pos.y -= dist;
  //       curr_depth++;
  //     }
  //   }
  // }

  if (!load_keys_from_config("basic_keys.ini")) { assert(false); };
}

void avbd_test_state::update() {
  PA::inst().start("update");

  // updating scalers
  {
    scalers::set("fps", 1.f / dt);
    scalers::set(
      "collider number",
      get_qt<poly_qt>()
        .size() // + get_qt<particle_qt>().size() + get_qt<liquid_qt>().size()
    );
    scalers::set(
      "contact number",
      get<cp6::pcnt_system>(m_phy).get_pcnt_data<poly_qt, poly_qt>(reg).size()

    );
    cp6::cnt_friction_coef = scalers::get(
      "friction coef",
      cp6::cnt_friction_coef,
      0.1
    );
    // cp6::imp_friction_coef = cp6::cnt_friction_coef;
    // cp6::cnt_k = scalers::get("k", cp6::cnt_k, 10'000);
    cp6::cnt_col_thresh = scalers::get(
      "collision thresh",
      cp6::cnt_col_thresh,
      0.1
    );
    cp6::cnt_stick_thresh = scalers::get(
      "stick thresh",
      cp6::cnt_stick_thresh,
      0.01
    );
    // cp6::cnt_gamma = scalers::get("pcnt gamma", cp6::cnt_gamma, (1 -
    // cp6::cnt_gamma)); cp6::pcnt_alpha = scalers::get("pcnt alpha",
    // cp6::pcnt_alpha, 1 - cp6::pcnt_alpha); cp6::cnt_beta = scalers::get("pcnt
    // beta", cp6::cnt_beta, cp6::cnt_beta); cp6::bounce =
    // scalers::get("bounce", cp6::bounce, 0.1);

    cp6::liquid_alpha = scalers::get("liquid_alpha", cp6::liquid_alpha, 0.01);
    cp6::liquid_beta = scalers::get(
      "liquid_beta",
      cp6::liquid_beta,
      cp6::liquid_beta
    );
    cp6::liquid_gamma = scalers::get("liquid_gamma", cp6::liquid_gamma, 0.01);
    cp6::liquid_init_k = scalers::get(
      "liquid_init_k",
      cp6::liquid_init_k,
      cp6::liquid_init_k
    );
    cp6::liquid_kernel_rad = scalers::get("h_rad", cp6::liquid_kernel_rad, 1);
    cp6::liquid_target_rho = scalers::get(
      "target_rho",
      cp6::liquid_target_rho,
      cp6::liquid_target_rho / 10.f
    );
    cp6::liquid_avbd = flags::get("liquid avbd", cp6::liquid_avbd);

    // setting col state constants
    {
      get<cp6::avbd_pcnt_fg>(m_phy)
        .get_avbd_pcnt_data<poly_qt, poly_qt>(reg)
        .alpha = scalers::get("pp_alpha", .90, 0.01);
      // get<cp6::avbd_tcnt_fg>(m_phy).get_avbd_tcnt_data<liquid_qt,
      // poly_qt>(reg).alpha =
      //   scalers::get("lp_alpha", .55, 0.01);
    }
  }

  // user input
  {
    // quit
    if (keybinds["Q"]->ispressed) { this->background_state = Remove; }

    // reset
    if (keybinds["R"]->ispressed) {
      this->background_state = Remove;
      m_man.push_new_state<avbd_test_state>();
    }

    // delete
    if (keybinds["Z"]->ispressed || keybinds["Z"]->timeheld > 0.1) {
      if (size<poly_qt>(get<reg_t>(m_phy), reg) > 4) {
        unsigned i = random_index(4, size<poly_qt>(get<reg_t>(m_phy), reg) - 1);
        std::cout << "removing: " << i << std::endl;
        null<poly_qt>(get<reg_t>(m_phy), {reg, i});
      }
      clear_null(get<reg_t>(m_phy));
    }

    // swap
    if (keybinds["S"]->ispressed || keybinds["S"]->timeheld > 0.1) {
      if (size<poly_qt>(get<reg_t>(m_phy), reg) > 4) {
        unsigned i = random_index(4, size<poly_qt>(get<reg_t>(m_phy), reg) - 1);
        unsigned j = random_index(4, size<poly_qt>(get<reg_t>(m_phy), reg) - 1);
        std::cout << "swapping: " << i << " <-> " << j << std::endl;
        swap<poly_qt>(get<reg_t>(m_phy), {reg, i}, {reg, j});
      }
      if (!get<cp6::color_system>(m_phy).check_sorted(reg)) {
        std::cout << "not sorted!" << std::endl;
      }
    }

    // camera movement
    {
      if (keybinds["Up"]->isheld) {
        m_view.setCenter(
          m_view.getCenter() - sf::Vector2f{0, 1 * dt * m_view.getSize().x}
        );
      }

      if (keybinds["Down"]->isheld) {
        m_view.setCenter(
          m_view.getCenter() + sf::Vector2f{0, 1 * dt * m_view.getSize().x}
        );
      }

      if (keybinds["Right"]->isheld) {
        m_view.setCenter(
          m_view.getCenter() + sf::Vector2f{2 * dt * m_view.getSize().y, 0}
        );
      }

      if (keybinds["Left"]->isheld) {
        m_view.setCenter(
          m_view.getCenter() - sf::Vector2f{2 * dt * m_view.getSize().y, 0}
        );
      }

      if (keybinds["Hyphen"]->isheld) { m_view.zoom(1 + dt); }

      if (keybinds["Equal"]->isheld) { m_view.zoom(1 - dt); }
    }

    const sf::RenderWindow &window = m_man.get_window();
    vec2f mouse_pos = from_sf(
      window.mapPixelToCoords(sf::Mouse::getPosition(window), m_view)
    );

    // basic mouse interaction
    // {
    //   if (get_qt<poly_qt>().size() > 1) {
    //     get_qt<poly_qt>().q()[0].xy() = mouse_pos;
    //   }
    //
    //   if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
    //     const std::vector<dlm::ikey<1>> &mouse_to_a_pcnt =
    //       get<cp6::pcnt_system>(m_phy).q1_to_pcnt<poly_qt, poly_qt>(reg, 0);
    //     const std::vector<dlm::ikey<1>> &mouse_to_b_pcnt =
    //       get<cp6::pcnt_system>(m_phy).q2_to_pcnt<poly_qt, poly_qt>(reg, 0);
    //
    //     unsigned other_q = -1;
    //     if (!mouse_to_a_pcnt.empty()) {
    //       other_q =
    //         get<cp6::pcnt_system>(m_phy).pcnt_to_q2<poly_qt,
    //         poly_qt>(reg)[mouse_to_a_pcnt[0]];
    //     } else if (!mouse_to_b_pcnt.empty()) {
    //       other_q =
    //         get<cp6::pcnt_system>(m_phy).pcnt_to_q1<poly_qt,
    //         poly_qt>(reg)[mouse_to_b_pcnt[0]];
    //     }
    //
    //     if (other_q != -1) {
    //       get_qt<poly_qt>().q()[other_q].xy() = mouse_pos;
    //       get_qt<poly_qt>().q_dot()[other_q].xy() = {0, 0};
    //     }
    //   }
    // }

    {
      if (get_qt<poly_qt>().size() > 1) {
        get_qt<poly_qt>().q()[0].xy() = mouse_pos;
      }

      if (is_mouse_grabbing == false) {
        if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
          const std::vector<dlm::ikey<1>>
            &mouse_to_a_pcnt = get<cp6::pcnt_system>(m_phy)
                                 .q1_to_pcnt<poly_qt, poly_qt>(reg, 0);
          const std::vector<dlm::ikey<1>>
            &mouse_to_b_pcnt = get<cp6::pcnt_system>(m_phy)
                                 .q2_to_pcnt<poly_qt, poly_qt>(reg, 0);

          unsigned other_q = -1;
          if (!mouse_to_a_pcnt.empty()) {
            other_q = get<cp6::pcnt_system>(m_phy).pcnt_to_q2<poly_qt, poly_qt>(
              reg
            )[mouse_to_a_pcnt[0]];
          } else if (!mouse_to_b_pcnt.empty()) {
            other_q = get<cp6::pcnt_system>(m_phy).pcnt_to_q1<poly_qt, poly_qt>(
              reg
            )[mouse_to_b_pcnt[0]];
          }

          if (other_q != -1) {
            assert(false);
            vec2f diff = get_qt<poly_qt>().q()[other_q].xy() - mouse_pos;
            get<cp6::avbd_pin_fg>(m_phy).add_pin<poly_qt, poly_qt>(
              reg,
              0,
              other_q,
              {.ra = {0, 0}, .rb = cp6::inv_transform_point(other_q, get_qt<poly_qt>(), mouse_pos)}
            );
            is_mouse_grabbing = true;
          }
        }
      }

      if (
        !sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)
        && is_mouse_grabbing
      ) {
        is_mouse_grabbing = false;
        unsigned pin = get<cp6::avbd_pin_fg>(m_phy).q1_to_pf<poly_qt, poly_qt>(
          reg,
          0
        )[0];

        get<cp6::avbd_pin_fg>(m_phy).remove_pf<poly_qt, poly_qt>(reg, pin);
      }
    }

    // adding shapes at mouse position
    {
      if (scalers::get("shape type", 1, 1) > 6) {
        scalers::set("shape type", 0);
      }
      // add shape
      if (keybinds["E"]->ispressed || keybinds["E"]->timeheld > 0.2) {
        unsigned i = scalers::get("shape type");
        if (i == 0) {
          add_poly(mouse_pos, vector_poly::square_shape(.3, .3));
        } else if (i == 1) {
          add_poly(mouse_pos, vector_poly::hexagon_shape(.5, .5));
        } else if (i == 2) {
          add_poly(mouse_pos, vector_poly::pentagon_shape(.4, .4));
        } else if (i == 3) {
          add_poly(mouse_pos, vector_poly::square_shape());
        } else if (i == 4) {
          unsigned q = add_poly(mouse_pos, vector_poly::square_shape(2, 2));
          get_qt<poly_qt>().mass()[q] *= 1000;
        } else if (i == 5) {
          add_particle(mouse_pos, 20);
        } else if (i == 6) {
          add_liquid_particle(mouse_pos);
        }
      }
    }
  }

  // physic update
  if (keybinds["T"]->ispressed || keybinds["T"]->timeheld > 0.1 || true) {
    const float iterations = scalers::get("iterations", 100, 1);
    const float sub_steps = scalers::get("sub steps", 10, 1);
    const float impulse_sub_steps = scalers::get("impulse sub steps", 4, 1);

    const float h = 1.f / iterations;

    // caller.update(dt, h, [&] {
    // transforming collider data
    {
      PA::inst().start("transform");
      for_each_i<qt_num>([&]<size_t QT> {
        cd_transform_start(get_qt<QT>().cd());
        cd_transform(get_qt<QT>().cd(), get_qt<QT>().qd());
        cd_transform_end(get_qt<QT>().cd());
      });
      PA::inst().end("transform");
    }

    PA::inst().start("bp");
    get<cp6::basic_broadphase>(m_phy).update(reg);
    PA::inst().end("bp");

    PA::inst().start("pl update");
    get<cp6::phase_link_system>(m_phy).update(reg);
    PA::inst().end("pl update");

    PA::inst().start("np");
    get<cp6::narrowphase_system>(m_phy).update(reg);
    PA::inst().end("np");

    // moving contacts so the liquid radius seems smaller to rigid bodies
    {
      auto &lp_tcnt_col_state = get<cp6::narrowphase_system>(m_phy)
                                  .get_tcnt_data<liquid_qt, poly_qt>(reg);
      for (unsigned i = 0; i < lp_tcnt_col_state.size(); i++) {
        lp_tcnt_col_state[i].x2 += lp_tcnt_col_state[i].n
                                 * (liquid_kernal_collider_radius + 10);
      }
    }

    // must be updated before resolving collisions
    PA::inst().start("pcnt update");
    get<cp6::pcnt_system>(m_phy).update();
    PA::inst().end("pcnt update");

    PA::inst().start("color update");
    get<cp6::color_system>(m_phy).update(reg);
    PA::inst().end("color update");

    // for_each_i<qt_num>([&]<size_t QT> {
    //   if constexpr (cp6::is_cqt<qt_t<QT>>::value) {
    //
    //     const auto &color_s = get<cp6::color_system>(m_phy);
    //     for (unsigned c = 0; c < color_s.color_num(reg); c++) {
    //       const basic_color_data &color = color_s.get_color_data<QT>(reg, c);
    //       std::cout << "c: "<< c <<"\n  ";
    //       for (unsigned cq = 0; cq < color.cq_num; cq++) {
    //         std::cout << color.to_q[cq] << " ";
    //       }
    //       std::cout << std::endl;
    //     }
    //   }
    // });

    // physics solve
    PA::inst().start("phy solve");
    {
      // adding gravity
      {
        for (unsigned q = 0; q < get_qt<poly_qt>().size(); q++) {
          get_qt<poly_qt>().a_ext()[q].y += 981;
        }

        for (unsigned q = 0; q < get_qt<particle_qt>().size(); q++) {
          get_qt<particle_qt>().a_ext()[q].y += 981;
        }

        for (unsigned q = 0; q < get_qt<liquid_qt>().size(); q++) {
          get_qt<liquid_qt>().a_ext()[q].y += 981;
        }
      }

      if (!flags::get("temporal sub steps", false)) {
        get<cp6::avbd_system>(m_phy).start_step(reg, h);

        for (unsigned i = 0; i < sub_steps; i++) {
          get<cp6::avbd_system>(m_phy).step(reg, h);
        }

        // impulses
        // {
        //   get<cp6::impulse_system>(m_phy).start_step(reg, h);
        //   for (unsigned i = 0; i < impulse_sub_steps; i++) {
        //     get<cp6::impulse_system>(m_phy).step(reg, h);
        //   }
        //   get<cp6::impulse_system>(m_phy).end_step(reg, h);
        // }

        get<cp6::avbd_system>(m_phy).end_step(reg, h);
      } else {
        for (unsigned i = 0; i < sub_steps; i++) {
          for (unsigned q = 0; q < get_qt<poly_qt>().size(); q++) {
            get_qt<poly_qt>().a_ext()[q].y = 981;
          }

          for (unsigned q = 0; q < get_qt<particle_qt>().size(); q++) {
            get_qt<particle_qt>().a_ext()[q].y = 981;
          }
          //
          // for (unsigned q = 0; q < get_qt<particle_qt>().size(); q++) {
          //   get_qt<liquid_qt>().a_ext()[q].y = 981;
          // }
          get<cp6::avbd_system>(m_phy).start_step(reg, h / sub_steps);
          get<cp6::avbd_system>(m_phy).step(reg, h / sub_steps);
          get<cp6::avbd_system>(m_phy).end_step(reg, h / sub_steps);
        }
      }
      // clamping rotation
      for (unsigned q = 0; q < get_qt<poly_qt>().size(); q++) {
        get_qt<poly_qt>().q_dot()[q].z = clamp(
          get_qt<poly_qt>().q_dot()[q].z,
          -5,
          5
        );
      }
    }
    PA::inst().end("phy solve");
    PA::inst().end("update");
    // });
  }

  // printing performance info every second
  static float elapsed = 0;
  elapsed += dt;
  if (elapsed > 1 && flags::get("print perf", false)) {
    PA::inst().print_all();
    std::cout << std::endl;

    {
      unsigned total_num = 0;
      for_each_i<qt_num>([&]<size_t QT> {
        if constexpr (cp6::is_cqt<qt_t<QT>>::value) {
          total_num += get_qt<QT>().size();
        }
      });

      auto &color_s = get<cp6::color_system>(m_phy);
      for (unsigned c = 0; c < color_s.color_num(reg); c++) {
        unsigned sum = 0;
        for_each_i<qt_num>([&]<size_t QT> {
          if constexpr (cp6::is_cqt<qt_t<QT>>::value) {
            const basic_color_data &color = color_s.get_color_data<QT>(reg, c);
            sum += color.cq_num;
          }
        });
        std::cout << "color " << c << " contains " << sum << " coordinates. ("
                  << ((float)sum / (float)total_num) * 100 << "%)" << std::endl;
      }
    }
    elapsed = 0;
  }
}

void avbd_test_state::draw(sf::RenderWindow &window) {
  // transforming collider data
  {

    PA::inst().start("transform");
    for_each_i<qt_num>([&]<size_t QT> {
      cd_transform_start(get_qt<QT>().cd());
      cd_transform(get_qt<QT>().cd(), get_qt<QT>().qd());
      cd_transform_end(get_qt<QT>().cd());
    });

    PA::inst().end("transform");
  }

  PA::inst().start("draw");
  // drawing cds
  for_each_i<qt_num>([&]<size_t QT> {
    if (QT != liquid_qt) {
      draw_cd(window, get_qt<QT>().cd(), sf::Color{75, 0, 110, 200});
    }
  });

  if (flags::get("draw_colors", true)) {
    auto &color_s = get<cp6::color_system>(m_phy);
    for_each_i<qt_num>([&]<size_t QT> {
      if constexpr (cp6::is_cqt<qt_t<QT>>::value) {
        for (unsigned c = 0; c < color_s.color_num(reg); c++) {
          const basic_color_data &color = color_s.get_color_data<QT>(reg, c);
          sf::Color curr_color;
          if (c < color_color_num) {
            curr_color = color_colors[c];
          } else {
            curr_color = sf::Color::White;
          }
          draw_cd_outline(
            window,
            get_qt<QT>().cd(),
            color.to_q,
            color.cq_num,
            curr_color
          );
        }
      }
    });
  }

  // drawing pins
  tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
    if constexpr (
      cp6::is_pinqt<qt_t<QT1>>::value && cp6::is_pinqt<qt_t<QT1>>::value
    ) {
      sf::CircleShape pin_circle;
      pin_circle.setRadius(3);
      pin_circle.setOrigin({pin_circle.getRadius(), pin_circle.getRadius()});
      pin_circle.setFillColor(sf::Color::White);

      const unsigned *edges_a = get<cp6::avbd_pin_fg>(m_phy).pf_to_q1<QT1, QT2>(
        reg
      );
      const unsigned *edges_b = get<cp6::avbd_pin_fg>(m_phy).pf_to_q2<QT1, QT2>(
        reg
      );
      const dlm::vec_ds<cp6::pin_data>
        &pin_state = get<cp6::avbd_pin_fg>(m_phy).get_pf_state<QT1, QT2>(reg);

      for (unsigned p = 0; p < pin_state.size(); p++) {
        unsigned q_a = edges_a[p];
        unsigned q_b = edges_b[p];

        vec2f pin_world_a = cp6::transform_point(
          q_a,
          get_qt<QT1>(),
          pin_state[p].ra
        );
        vec2f pin_world_b = cp6::transform_point(
          q_b,
          get_qt<QT2>(),
          pin_state[p].rb
        );

        pin_circle.setPosition(to_sf(pin_world_a));
        window.draw(pin_circle);
        pin_circle.setPosition(to_sf(pin_world_b));
        window.draw(pin_circle);
      }
    }
  });

  // drawing contacts
  if (flags::get("draw contacts", false)) {
    // drawing persistant contacts green
    tri_apply<qt_num>([&]<size_t QT_A, size_t QT_B> {
      if constexpr (
        cp6::is_pcqt<qt_t<QT_A>>::value && cp6::is_pcqt<qt_t<QT_B>>::value
      ) {
        draw_col_state(
          window,
          get<cp6::pcnt_system>(m_phy).template get_pcnt_data<QT_A, QT_B>(reg),
          {0, 0},
          sf::Color::Green,
          sf::Color::Green
        );
      }
    });

    // drawing temporary contacts red and blue
    tri_apply<qt_num>([&]<size_t QT1, size_t QT2> {
      draw_col_state(
        window,
        get<cp6::narrowphase_system>(m_phy).template get_tcnt_data<QT1, QT2>(
          reg
        ),
        {0, 0}
      );
    });

    // drawing sticking contacts yellow
    {
      const auto &avbd_cnts = get<cp6::avbd_pcnt_fg>(m_phy)
                                .get_avbd_pcnt_data<poly_qt, poly_qt>(reg);
      sf::CircleShape p;
      p.setRadius(5);
      p.setOutlineThickness(-1);
      p.setOrigin({p.getRadius(), p.getRadius()});
      p.setFillColor(sf::Color::Yellow);
      for (unsigned i = 0; i < avbd_cnts.size(); i++) {
        const auto &c = avbd_cnts[i];
        if (c.cnt1.stick) {
          vec2f world1 = transform_point(
            c.q1,
            get_qt<poly_qt>().qd(),
            c.cnt1.x1
          );
          vec2f world2 = transform_point(
            c.q2,
            get_qt<poly_qt>().qd(),
            c.cnt1.x2
          );
          p.setPosition(to_sf(world1));
          window.draw(p);
          p.setPosition(to_sf(world2));
          window.draw(p);
        }
        if (c.cnt2.is_valid() && c.cnt2.stick) {
          vec2f world1 = transform_point(
            c.q1,
            get_qt<poly_qt>().qd(),
            c.cnt2.x1
          );
          vec2f world2 = transform_point(
            c.q2,
            get_qt<poly_qt>().qd(),
            c.cnt2.x2
          );
          p.setPosition(to_sf(world1));
          window.draw(p);
          p.setPosition(to_sf(world2));
          window.draw(p);
        }
      }
    }
  }

  // drawing liquid qt
  {
    auto &lqt = get_qt<liquid_qt>();
    sf::CircleShape point;
    point.setRadius(liquid_particle_radius);
    point.setOrigin({point.getRadius(), point.getRadius()});
    // point.setFillColor({173, 0, 255, 150});
    point.setFillColor({0, 120, 255, 150});

    for (unsigned q = 0; q < lqt.size(); q++) {
      point.setPosition(to_sf(lqt.q()[q]));
      window.draw(point);
    }
  }

  // drawing body forces
  if (flags::get("draw body forces", false)) {
    auto &pqt = get_qt<poly_qt>();
    std::array<sf::Vertex, 2> arrow;
    for (unsigned q = 0; q < pqt.size(); q++) {
      arrow[0].color = {255, 255, 0, 200};
      arrow[1].color = {255, 0, 0, 200};
      arrow[0].position = to_sf(pqt.q()[q].xy());
      arrow[1].position = to_sf(
        pqt.q()[q].xy() - pqt.f_sum()[q].xy() * 5 / 1e7
      );
      window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
      arrow[0].color = {0, 255, 0, 200};
      arrow[1].color = {0, 255, 255, 200};
      float f_len = length(pqt.f_sum()[q].xy());
      if (f_len > 0) {
        arrow[0].position = to_sf(pqt.q()[q].xy());
        arrow[1].position = to_sf(
          pqt.q()[q].xy()
          + perp_l(pqt.f_sum()[q].xy() / f_len) * (pqt.f_sum()[q].z / 1e7)
        );
      }
      window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
    }
  }
  // drawing contact lambda
  if (flags::get("draw contact lambda", false)) {
    std::array<sf::Vertex, 2> arrow;
    auto &pqd = get_qt<poly_qt>().qd();
    const auto &avbd_cnts = get<cp6::avbd_pcnt_fg>(m_phy)
                              .get_avbd_pcnt_data<poly_qt, poly_qt>(reg);
    for (unsigned i = 0; i < avbd_cnts.size(); i++) {
      const auto &c = avbd_cnts[i];
      if (c.cnt1.stick) {
        arrow[0].color = {255, 255, 0, 200};
        arrow[1].color = {255, 0, 0, 200};
        arrow[0].position = to_sf(transform_point(c.q1, pqd, c.cnt1.x1));
        arrow[1].position = to_sf(transform_point(
          c.q1,
          pqd,
          c.cnt1.x1 + c.cnt1.n * (c.cnt1.lambda.x / 1e7)
        ));
        window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
      }
      if (c.cnt2.is_valid()) {
        arrow[0].color = {255, 255, 0, 200};
        arrow[1].color = {255, 0, 0, 200};
        arrow[0].position = to_sf(transform_point(c.q1, pqd, c.cnt2.x1));
        arrow[1].position = to_sf(transform_point(
          c.q1,
          pqd,
          c.cnt2.x1 + c.cnt2.n * (c.cnt2.lambda.x / 1e7)
        ));
        window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
      }
    }
  }

  PA::inst().end("draw");
}

unsigned avbd_test_state::add_poly(vec2f pos, vector_poly shape) {
  // std::cout << "adding poly at position: " << pos << std::endl;
  unsigned q = get_qt<poly_qt>().size();
  grow<poly_qt>(get<reg_t>(m_phy), 1, {reg});
  get_qt<poly_qt>().q()[q].xy() = pos;
  get_qt<poly_qt>().base_cd()[q] = std::move(shape);
  get_qt<poly_qt>().calculate_constants(
    q,
    &get_qt<poly_qt>().mass()[q].x,
    &get_qt<poly_qt>().mass()[q].z
  );
  get_qt<poly_qt>().mass()[q].y = get_qt<poly_qt>().mass()[q].x;

  return q;
}

unsigned avbd_test_state::add_particle(vec2f pos, float radius) {
  auto &qt = get_qt<particle_qt>();
  unsigned q = qt.size();
  grow<particle_qt>(get<reg_t>(m_phy), 1, {reg});
  qt.q()[q] = pos;
  qt.cd()[q].radius = radius;
  get_qt<particle_qt>().cd().calculate_constants(q, &qt.mass()[q].x);
  qt.mass()[q].y = qt.mass()[q].x;
  return q;
}

unsigned avbd_test_state::add_liquid_particle(vec2f pos) {
  auto &qt = get_qt<liquid_qt>();
  unsigned q = qt.size();
  grow<liquid_qt>(get<reg_t>(m_phy), 1, {reg});
  qt.q()[q] = pos;
  qt.cd()[q].radius = liquid_particle_radius;
  get_qt<liquid_qt>().cd().calculate_constants(q, &qt.mass()[q].x);
  qt.mass()[q].y = qt.mass()[q].x;
  qt.mass()[q] *= cp6::lms;

  qt.cd()[q].radius = liquid_kernal_collider_radius;
  return q;
}
