#pragma once
#include "collision_basic/contact_set.h"
#include "ds_utils.h"
#include "collision_basic/poly_cd.h"

namespace cp6 {

template<typename CD1, typename CD2>
struct imp_combo;

template<typename CD1, typename CD2>
using imp_combo_t = imp_combo<CD1, CD2>::type;

struct impulse_contact: contact {
  float total_impulse_n = 0;
  float total_impulse_t = 0;
  float rel_v_start = 0;
};

struct impulse_contact_pair {
  impulse_contact impulse_a;
  impulse_contact impulse_b;
};

struct imp_data_2p: dlm::dst<dlm::vec_ds<impulse_contact_pair>> {
  DEFINE_DS_FIELD((0), impulses);
};

template<>
struct imp_combo<cp6::poly_cd, cp6::poly_cd> {
  using type = imp_data_2p;
};
} // namespace cp6
