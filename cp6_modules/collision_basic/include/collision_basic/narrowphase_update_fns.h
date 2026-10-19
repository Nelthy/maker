#pragma once
#include "circle_cd.h"
#include "contact_set.h"
#include "poly_cd.h"

namespace cp6 {
bool narrowphase_update(
  const cp6::circle_cd &collider_data_a,
  const cp6::circle_cd &collider_data_b,
  cp6::contact_1p_set &collision_state

);

bool narrowphase_update(
  const cp6::circle_cd &collider_data_a,
  const cp6::poly_cd &collider_data_b,
  contact_1p_set &collision_state

);

bool narrowphase_update(
  const cp6::poly_cd &collider_data_a,
  const cp6::poly_cd &collider_data_b,
  cp6::contact_2p_set &collision_state
);
} // namespace cp6
