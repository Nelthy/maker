#pragma once

namespace cp6 {
// template<typename CD1_T, typename CD2_T, typename POTCOLS_T, typename COLS_T>
// bool narrowphase(const CD1_T &cd1, const CD2_T &cd2, const POTCOLS_T &potcols, COLS_T &dst_cols);
//
// template<typename CD1_T, typename CD2_T, typename COLS_T>
// bool narrowphase_update(const CD1_T &cd1, const CD2_T &cd2, COLS_T &dst_cols);

template<typename TCNTS_T, typename PCNTS_T>
bool copy_tcnt_to_pcnt(const TCNTS_T &tcnts, unsigned pcnt_start, PCNTS_T &pcnts);

template<typename PHASE_LINKS_T>
bool add_pcnt_phase_links(
  unsigned pcnt_num,
  const unsigned *pcnt_to_qt1,
  const unsigned *pcnt_to_qt2,
  PHASE_LINKS_T &phase_links
);

template<typename PCNTS_T>
bool update_pcnt_edges(const unsigned *pcnt_to_q1, const unsigned *pcnt_to_q2, PCNTS_T &pcnts);
} // namespace cp6
