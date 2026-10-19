#pragma once
#include "bin_table.h"
#include "collision_basic/contact_set.h"
#include "tri_bin_table.h"

bool add_pcnt_phase_links(
  unsigned pcnt_num,
  const unsigned *pcnt_to_qt1,
  const unsigned *pcnt_to_qt2,
  tri_bin_table &phase_links
);

bool add_pcnt_phase_links(
  unsigned pcnt_num,
  const unsigned *pcnt_to_qt1,
  const unsigned *pcnt_to_qt2,
  bin_table &phase_links
);

namespace cp6 {
bool copy_tcnt_to_pcnt(
  const cp6::contact_1p_set &tcnts,
  unsigned pcnt_start,
  cp6::contact_1p_set &pcnts
);

bool copy_tcnt_to_pcnt(
  const cp6::contact_2p_set &tcnts,
  unsigned pcnt_start,
  cp6::contact_2p_set &pcnts
);

bool update_pcnt_edges(
  const unsigned *pcnt_to_q1,
  const unsigned *pcnt_to_q2,
  cp6::contact_1p_set &pcnts
);

bool update_pcnt_edges(
  const unsigned *pcnt_to_q1,
  const unsigned *pcnt_to_q2,
  cp6::contact_2p_set &pcnts
);

} // namespace cp6
