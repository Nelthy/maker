#pragma once

// tcnt = temporary contact
template<typename CD1_T, typename CD2_T>
struct tcnt_combo;

template<typename CD1_T, typename CD2_T>
using tcnt_combo_t = typename tcnt_combo<CD1_T, CD2_T>::type;

#define SET_TCNT_COMBO(cd1, cd2, state_type)                                                       \
  template<>                                                                                       \
  struct tcnt_combo<cd1, cd2> {                                                                    \
    using type = state_type;                                                                       \
  };

// pcol = potential collision
template<typename CD1_T, typename CD2_T>
struct pcol_combo;

template<typename CD1_T, typename CD2_T>
using pcol_combo_t = typename pcol_combo<CD1_T, CD2_T>::type;

#define SET_PCOL_COMBO(cd1, cd2, state_type)                                                       \
  template<>                                                                                       \
  struct pcol_combo<cd1, cd2> {                                                                    \
    using type = state_type;                                                                       \
  };

// pcnt = persistant contact
template<typename CD1_T, typename CD2_T>
struct pcnt_combo;

template<typename CD1_T, typename CD2_T>
using pcnt_combo_t = typename pcnt_combo<CD1_T, CD2_T>::type;

#define SET_PCNT_COMBO(cd1, cd2, state_type)                                                       \
  template<>                                                                                       \
  struct pcnt_combo<cd1, cd2> {                                                                    \
    using type = state_type;                                                                       \
  };

template<typename CD1_T, typename CD2_T>
struct phase_links_combo;

template<typename CD1_T, typename CD2_T>
using phase_links_combo_t = typename phase_links_combo<CD1_T, CD2_T>::type;

#define SET_PHASE_LINKS_COMBO(cd1, cd2, state_type)                                                \
  template<>                                                                                       \
  struct phase_links_combo<cd1, cd2> {                                                             \
    using type = state_type;                                                                       \
  };

template<typename CD_T>
struct phase_links_self;

template<typename CD_T>
using phase_links_self_t = typename phase_links_self<CD_T>::type;

#define SET_PHASE_LINKS_SELF(cd1, state_type)                                                      \
  template<>                                                                                       \
  struct phase_links_self<cd1> {                                                                   \
    using type = state_type;                                                                       \
  };

template<typename QT1, typename QT2>
struct can_qts_collide_combo {
  // can be overridden by user
  static constexpr bool value = true;
};

template<typename QT1, typename QT2>
static constexpr bool can_qts_collide_combo_v = can_qts_collide_combo<QT1, QT2>::value;

#define DISABLE_QT_COLLISIONS(qt1_t, qt2_t)                                                        \
  template<>                                                                                       \
  struct can_qts_collide_combo<qt1_t, qt2_t> {                                                     \
    static constexpr bool value = false;                                                           \
  };
