#pragma once
#include "data_set_tuple.h"
#include "vec_ds.h"
template<typename CD1, typename CD2>
struct avbd_liquid_state;

template<typename CD1, typename CD2>
using avbd_liquid_state_t = typename avbd_liquid_state<CD1, CD2>::type;

#define SET_AVBD_LIQUID_STATE(cd1_t, cd2_t, liquid_state_t)                                        \
  template<>                                                                                       \
  struct avbd_liquid_state<cd1_t, cd2_t> {                                                         \
    using type = liquid_state_t;                                                                   \
  };

struct liquid_state: dlm::dst<dlm::vec_ds<float>> {};
