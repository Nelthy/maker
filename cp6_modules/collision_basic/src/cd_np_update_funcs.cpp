#include "collision_basic/narrowphase_fns.h"
#include "collision_basic/narrowphase_update_fns.h"

namespace cp6 {
bool narrowphase_update(
  const circle_cd &collider_data_a,
  const circle_cd &collider_data_b,
  contact_1p_set &collision_state
) {
  unsigned valid_cnt_num = 0;
  for (unsigned cnt = 0; cnt < collision_state.size(); cnt++) {
    collision_state[cnt].invalidate();
    cp6::cvc_np(
      collider_data_a,
      collider_data_b,
      collision_state[cnt].q1,
      collision_state[cnt].q2,
      collision_state[cnt]
    );
    // setting q1 to -1 tells the pcnt system to remove this contact
    if (!collision_state[cnt].is_valid()) {
      collision_state[cnt].q1 = -1;
    }
  }
  return true;
};

bool narrowphase_update(
  const circle_cd &collider_data_a,
  const poly_cd &collider_data_b,
  contact_1p_set &collision_state
) {
  unsigned valid_cnt_num = 0;
  for (unsigned cnt = 0; cnt < collision_state.size(); cnt++) {
    collision_state[cnt].invalidate();
    cp6::cvp_np(
      collider_data_a,
      collider_data_b,
      collision_state[cnt].q1,
      collision_state[cnt].q2,
      collision_state[cnt]
    );
    if (!collision_state[cnt].is_valid()) {
      collision_state[cnt].q1 = -1;
    }
  }
  return true;
};

bool narrowphase_update(
  const poly_cd &collider_data_a,
  const poly_cd &collider_data_b,
  contact_2p_set &collision_state
) {
  unsigned valid_cnt_num = 0;
  for (unsigned cnt = 0; cnt < collision_state.size(); cnt++) {
    collision_state[cnt].cnt1.invalidate();
    collision_state[cnt].cnt2.invalidate();
    cp6::pvp_np(
      collider_data_a,
      collider_data_b,
      collision_state[cnt].q1,
      collision_state[cnt].q2,
      collision_state[cnt].cnt1,
      collision_state[cnt].cnt2
    );

    if (!collision_state[cnt].cnt1.is_valid()) {
      collision_state[cnt].q1 = -1;
    }
  }
  return true;
};
} // namespace cp6
