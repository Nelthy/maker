#pragma once
#include "ab_link.h"
#include "circle_cd.h"
#include "contact_set.h"
#include "poly_cd.h"

namespace cp6 {
bool narrowphase(
  const cp6::circle_cd &collider_data_a,
  const cp6::circle_cd &collider_data_b,
  const std::vector<ab_link> &potential_collisions,
  contact_1p_set &dst_collision_state
);

bool narrowphase(
  const cp6::circle_cd &collider_data_a,
  const cp6::poly_cd &collider_data_b,
  const std::vector<ab_link> &potential_collisions,
  cp6::contact_1p_set &dst_collision_state
);

bool narrowphase(
  const cp6::poly_cd &collider_data_a,
  const cp6::poly_cd &collider_data_b,
  const std::vector<ab_link> &potential_collisions,
  cp6::contact_2p_set &dst_collision_state
);

void cvc_np(
  const circle_cd &cd1,
  const circle_cd &cd2,
  unsigned c1,
  unsigned c2,
  cp6::contact &dst_contact
);

void cvp_np(
  const circle_cd &cd1,
  const poly_cd &cd2,
  unsigned c1,
  unsigned c2,
  cp6::contact &dst_contact
);

void pvp_np(
  const poly_cd &cd1,
  const poly_cd &cd2,
  unsigned c1,
  unsigned c2,
  cp6::contact &dst_contact1,
  cp6::contact &dst_contact2
);
} // namespace cp6
