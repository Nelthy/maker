#include "collision_basic/pcnt_fns.h"

bool add_pcnt_phase_links(
  unsigned pcnt_num,
  const unsigned *pcnt_to_qt1,
  const unsigned *pcnt_to_qt2,
  tri_bin_table &phase_links
) {
  for (unsigned pcnt = 0; pcnt < pcnt_num; pcnt++) {
    unsigned q1 = pcnt_to_qt1[pcnt];
    unsigned q2 = pcnt_to_qt2[pcnt];
    if (q1 > q2) {
      std::swap(q1, q2);
    }
    phase_links.set(q1, q2, true);
  }
  return true;
}

bool add_pcnt_phase_links(
  unsigned pcnt_num,
  const unsigned *pcnt_to_qt1,
  const unsigned *pcnt_to_qt2,
  bin_table &phase_links
) {
  for (unsigned i = 0; i < pcnt_num; i++) {
    phase_links.set(pcnt_to_qt1[i], pcnt_to_qt2[i], true);
  }
  return true;
}

namespace cp6 {

bool copy_tcnt_to_pcnt(
  const cp6::contact_1p_set &tcnts,
  unsigned pcnt_start,
  cp6::contact_1p_set &pcnts
) {
  for (unsigned pcnt = pcnt_start; pcnt < pcnts.size(); pcnt++) {
    unsigned tcnt = pcnt - pcnt_start;
    pcnts[pcnt] = tcnts[tcnt];
  }
  return true;
}

bool copy_tcnt_to_pcnt(
  const cp6::contact_2p_set &tcnts,
  unsigned pcnt_start,
  cp6::contact_2p_set &pcnts
) {
  for (unsigned pcnt = pcnt_start; pcnt < pcnts.size(); pcnt++) {
    unsigned tcnt = pcnt - pcnt_start;
    pcnts[pcnt] = tcnts[tcnt];
  }
  return true;
}

bool update_pcnt_edges(
  const unsigned *pcnt_to_q1,
  const unsigned *pcnt_to_q2,
  cp6::contact_1p_set &pcnts
) {
  for (unsigned pcnt = 0; pcnt < pcnts.size(); pcnt++) {
    pcnts[pcnt].q1 = pcnt_to_q1[pcnt];
    pcnts[pcnt].q2 = pcnt_to_q2[pcnt];
  }
  return true;
};

bool update_pcnt_edges(
  const unsigned *pcnt_to_q1,
  const unsigned *pcnt_to_q2,
  cp6::contact_2p_set &pcnts
) {
  for (unsigned pcnt = 0; pcnt < pcnts.size(); pcnt++) {
    pcnts[pcnt].q1 = pcnt_to_q1[pcnt];
    pcnts[pcnt].q2 = pcnt_to_q2[pcnt];
  }
  return true;
};

} // namespace cp6
