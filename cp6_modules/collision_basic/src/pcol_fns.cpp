#include "collision_basic/pcol_fns.h"

bool cull_pcols(std::vector<ab_link> &pcols, const tri_bin_table &phase_links) {
  for (unsigned i = 0; i < pcols.size(); i++) {
    if (phase_links.get(pcols[i].a, pcols[i].b)) {
      pcols[i] = pcols.back();
      pcols.pop_back();
      i--;
    }
  }
  return true;
}

bool cull_pcols(std::vector<ab_link> &pcols, const bin_table &phase_links) {
  for (unsigned i = 0; i < pcols.size(); i++) {
    if (phase_links.get(pcols[i].a, pcols[i].b)) {
      pcols[i] = pcols.back();
      pcols.pop_back();
      i--;
    }
  }
  return true;
}
