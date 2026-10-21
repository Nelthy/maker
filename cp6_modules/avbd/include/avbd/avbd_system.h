#pragma once
#include "avbd_flags.h"
#include "color_system.h"
#include "god_state.h"
#include "region_state.h"
#include "tuple_utils.h"
#include <type_traits>

namespace cp6 {
template<typename TW>
struct avbd_system_tt {
  static constexpr auto vbd_qt_to_qt =
    generate_tuple_indirection<get_matching_ts<is_vbd_qt, qt_ts<TW>>, qt_ts<TW>>();

  static constexpr unsigned vbd_qt_num = get_matching_num<is_vbd_qt, qt_ts<TW>>();
  static_assert(vbd_qt_num > 0);

  void start_step(unsigned reg, float h) {
    auto fgs = get_matching<is_avbd_fg>(*m_state);
    auto &region_s = get<is_region_state>(*m_state);

    spin_loop_i<vbd_qt_num>([&]<size_t VBD_QT> {
      auto &qt_s = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
      return qt_s.start_step(h);
    });
    tuple_spin_loop([&](auto &fg) { return fg.start_step(reg, h); }, fgs);
  };

  void step(unsigned reg, float h) {
    auto jacobi_fgs = get_matching<is_avbd_jacobi_fg>(*m_state);
    auto gs_fgs = get_matching<is_avbd_gs_fg>(*m_state);
    auto fgs = get_matching<is_avbd_fg>(*m_state);
    auto &region_s = get<is_region_state>(*m_state);

    // initializing derivatives and adding Jacobi forces
    spin_loop_i<vbd_qt_num>([&]<size_t VBD_QT> {
      auto &qt_s = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
      return qt_s.init_derivs(h);
    });

    tuple_spin_loop([&](auto &jacobi_fg) { return jacobi_fg.add_derivs(reg); }, jacobi_fgs);

    // non-colorable newton step
    spin_loop_i<vbd_qt_num>([&]<size_t VBD_QT> {
      auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
      if constexpr (!requires { qt_state.colorable; }) {
        return qt_state.newton_step();
      } else {
        return true;
      }
    });

    auto &color_s = get<color_system>(*m_state);
    for (unsigned c = 0; c < color_s.color_num(reg); c++) {
      // adding Gauss Seidel forces
      tuple_spin_loop([&](auto &gs_fg) { return gs_fg.add_derivs(reg, c); }, gs_fgs);

      // colorable newton step
      spin_loop_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_s = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (requires { qt_s.colorable; }) {
          return qt_s.newton_step(color_s.template get_color_data<VBD_QT>(reg, c));
        } else {
          return true;
        }
      });
    }

    // after step
    tuple_spin_loop([&](auto &fg) { return fg.after_step(reg, h); }, fgs);
  }

  void end_step(unsigned reg, float h) {

    auto fgs = get_matching<is_avbd_fg>(*m_state);
    auto &reg_s = get<is_region_state>(*m_state);

    spin_loop_i<vbd_qt_num>([&]<size_t VBD_QT> {
      auto &qt_s = reg_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
      return qt_s.end_step(h);
    });
    tuple_spin_loop([&](auto &fg) { return fg.end_step(reg, h); }, fgs);
  }

  void init(god::state<TW> &state) { m_state = &state; }

  god::state<TW> *m_state;
};

struct avbd_system {
  template<typename TW>
  using template_t = avbd_system_tt<TW>;
};
} // namespace cp6

/*
  THOUGHTS:
    Newton step adn vbd_start and vbd_end should just be member functions of the qd. No
  generalization would be lost, and we can query them with require clauses. This way we can easilly
  check if a qt has a parallel implmentaiton or not.

    This parallel bullshit is annoying.
    I don't want to have a centralized way of doing multithreading, so this is what I have to resort
  to.

    I hope it only makes writing this annoying. Implementation should be pretty smooth with the
  proper flag structs.

    Here are the flags:
      for force generators
        vbd_jacobi_fg
        vbd_gs_fg
        parallel_jacobi
        parallel_gs
        parallel_start_step
        parallel_end_step
        parallel_after_step
      for qts
        parallel_newton

   Okay, fuck this. A force gen or qt can be flagged as parallel, and if it is, it will have to
  define all the correct functions.


*/

/*

template<typename TW>
struct avbd_system_tt {
  static constexpr auto vbd_qt_to_qt = generate_tuple_indirection<
    get_matching_ts<is_vbd_qt, qt_ts<TW>>,
    qt_ts<TW>,
    std::is_same
  >();

  static constexpr unsigned vbd_qt_num = get_matching_num<is_vbd_qt, qt_ts<TW>>();
  static_assert(vbd_qt_num > 0);

  void start_step(unsigned reg, float h) {
    auto jacobi_fgs = get_matching<is_avbd_jacobi_fg>(*m_state);
    auto gs_fgs = get_matching<is_avbd_gs_fg>(*m_state);
    auto &region_s = get<is_region_state>(*m_state);
    // parallel start
    {
      tuple_for_each(
        [&](auto &jacobi_fg) {
          if constexpr (requires { jacobi_fg.parallel; }) jacobi_fg.start_step_p1(reg, h);
        },
        jacobi_fgs
      );

      tuple_for_each(
        [&](auto &gs_fg) {
          if constexpr (requires { gs_fg.parallel; } && !requires { gs_fg.vbd_jacobi_fg; })
            gs_fg.start_step_p1(reg, h);
        },
        gs_fgs
      );

      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (requires { qt_state.parallel; }) {
          vbd_start_step_p1(qt_state.qd(), h);
        }
      });
    }

    // serial
    {
      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (!requires { qt_state.parallel; }) {
          vbd_start_step(qt_state.qd(), h);
        }
      });

      tuple_for_each([&](auto &jacobi_fg) { jacobi_fg.start_step(reg, h); }, jacobi_fgs);
      tuple_for_each(
        [&](auto &gs_fg) {
          if constexpr (!requires { gs_fg.vbd_jacobi_fg; }) {
            gs_fg.start_step(reg, h);
          }
        },
        gs_fgs
      );
    }

    // parallel end
    {
      tuple_for_each(
        [&](auto &jacobi_fg) {
          if constexpr (requires { jacobi_fg.parallel; }) jacobi_fg.start_step_p2(reg, h);
        },
        jacobi_fgs
      );

      tuple_for_each(
        [&](auto &gs_fg) {
          if constexpr (requires { gs_fg.parallel; } && !requires { gs_fg.vbd_jacobi_fg; })
            gs_fg.start_step_p2(reg, h);
        },
        gs_fgs
      );

      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (requires { qt_state.parallel; }) {
          vbd_start_step_p2(qt_state.qd(), h);
        }
      });
    }
  };

  void step(unsigned reg, float h) {
    auto jacobi_fgs = get_matching<is_vbd_jacobi_fg>(*m_state);
    auto gs_fgs = get_matching<is_vbd_gs_fg>(*m_state);
    auto &region_s = get<is_region_state>(*m_state);

    // initializing derivatives and adding Jacobi forces
    {
      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        init_derivs(qt_state.qd(), h);
      });

      // parallel start
      tuple_for_each(
        [&](auto &jacobi_fg) {
          if constexpr (requires { jacobi_fg.parallel; }) {
            jacobi_fg.add_derivs_p1(reg);
          }
        },
        jacobi_fgs
      );

      tuple_for_each(
        [&](auto &jacobi_fg) {
          if constexpr (!requires { jacobi_fg.parallel; }) {
            jacobi_fg.add_derivs(reg);
          }
        },
        jacobi_fgs
      );

      // parallel end
      tuple_for_each(
        [&](auto &jacobi_fg) {
          if constexpr (requires { jacobi_fg.parallel; }) {
            jacobi_fg.add_derivs_p2(reg);
          }
        },
        jacobi_fgs
      );
    }

    // non-colorable newton step
    {
      // parallel start
      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (requires { qt_state.parallel; } && !requires { qt_state.colorable; }) {
          newton_step_p1(qt_state.qd());
        }
      });

      // serial
      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (!requires { qt_state.parallel; } && !requires { qt_state.colorable; }) {
          newton_step(qt_state.qd());
        }
      });

      // parallel end
      for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
        auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
        if constexpr (requires { qt_state.parallel; } && !requires { qt_state.colorable; }) {
          newton_step_p2(qt_state.qd());
        }
      });
    }

    auto &color_s = get<color_system>(*m_state);
    for (unsigned c = 0; c < color_s.color_num(reg); c++) {
      // adding Gauss Seidel forces
      {
        // parallel start
        tuple_for_each(
          [&](auto &gs_fg) {
            if constexpr (requires { gs_fg.parallel; }) {
              gs_fg.add_derivs_p1(reg, c);
            }
          },
          gs_fgs
        );

        // serial
        tuple_for_each(
          [&](auto &gs_fg) {
            if constexpr (!requires { gs_fg.parallel; }) {
              gs_fg.add_derivs(reg, c);
            }
          },
          gs_fgs
        );

        // parallel end
        for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
          static constexpr unsigned qt = vbd_qt_to_qt[VBD_QT];
          tuple_for_each(
            [&](auto &gs_fg) {
              if constexpr (requires { gs_fg.parallel; }) {
                gs_fg.add_derivs_p2(reg, c);
              }
            },
            gs_fgs
          );
        });
      }

      // colorable newton step
      {
        // parallel start
        for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
          auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
          if constexpr (requires { qt_state.parallel; } && requires { qt_state.colorable; }) {
            newton_step_p1(qt_state.qd(), color_s.template get_color_data<VBD_QT>(reg, c));
          }
        });

        // serial
        for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
          auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
          if constexpr (!requires { qt_state.parallel; } && requires { qt_state.colorable; }) {
            newton_step(qt_state.qd(), color_s.template get_color_data<VBD_QT>(reg, c));
          }
        });

        // parallel end
        for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
          auto &qt_state = region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg);
          if constexpr (requires { qt_state.parallel; } && requires { qt_state.colorable; }) {
            newton_step_p2(qt_state.qd(), color_s.template get_color_data<VBD_QT>(reg, c));
          }
        });
      }
    }

    // after step
    {
      {
        tuple_for_each(
          [&](auto &jacobi_fg) {
            if constexpr (requires { jacobi_fg.parallel; }) {
              jacobi_fg.after_step_p1(reg, h);
            }
          },
          jacobi_fgs
        );

        tuple_for_each(
          [&](auto &gs_fg) {
            if constexpr (!requires { gs_fg.vbd_jacobi_fg; } && requires { gs_fg.parallel; }) {
              gs_fg.after_step_p1(reg, h);
            }
          },
          gs_fgs
        );
      }

      // serial
      {
        tuple_for_each(
          [&](auto &jacobi_fg) {
            if constexpr (!requires { jacobi_fg.parallel; }) {
              jacobi_fg.after_step(reg, h);
            }
          },
          jacobi_fgs
        );

        tuple_for_each(
          [&](auto &gs_fg) {
            if constexpr (!requires { gs_fg.vbd_jacobi_fg; } && !requires { gs_fg.parallel; }) {
              gs_fg.after_step(reg, h);
            }
          },
          gs_fgs
        );
      }

      // p2
      {
        tuple_for_each(
          [&](auto &jacobi_fg) {
            if constexpr (requires { jacobi_fg.parallel; }) {
              jacobi_fg.after_step_p2(reg, h);
            }
          },
          jacobi_fgs
        );

        tuple_for_each(
          [&](auto &gs_fg) {
            if constexpr (!requires { gs_fg.vbd_jacobi_fg; } && requires { gs_fg.parallel; }) {
              gs_fg.after_step_p2(reg, h);
            }
          },
          gs_fgs
        );
      }
    }
  }

  void end_step(unsigned reg, float h) {
    auto jacobi_fgs = get_matching<is_vbd_jacobi_fg>(*m_state);
    auto gs_fgs = get_matching<is_vbd_gs_fg>(*m_state);
    auto &region_s = get<is_region_state>(*m_state);

    for_each_i<vbd_qt_num>([&]<size_t VBD_QT> {
      vbd_end_step(region_s.template get_qt<vbd_qt_to_qt[VBD_QT]>(reg).qd(), h);
    });
    tuple_for_each([&](auto &jacobi_fg) { jacobi_fg.end_step(reg, h); }, jacobi_fgs);
    tuple_for_each(
      [&](auto &gs_fg) {
        if constexpr (!requires { gs_fg.vbd_jacobi_fg; }) {
          gs_fg.end_step(reg, h);
        }
      },
      gs_fgs
    );
  }

  void init(god::state<TW> &state) { m_state = &state; }

  god::state<TW> *m_state;
};

struct avbd_system {
  template<typename TW>
  using template_t = avbd_system_tt<TW>;
};


*/
