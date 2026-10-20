#pragma once
#include "aabb.h"
#include "ab_link.h"
#include "overlapping_defs.h"
#include "vec_ds.h"
#include <vector>

// namespace cp6 {
struct sap_pcols: std::vector<ab_link> {
  std::vector<unsigned> sorted_to_unsorted;
};

// brute force for the fallback option
inline bool update_pcols(
  const auto &bd1,
  const auto &bd2,
  std::vector<ab_link> &pc
) {
  for (unsigned q1 = 0; q1 < bd1.size(); q1++) {
    auto b1 = bd1[q1];
    for (unsigned q2 = 0; q2 < bd2.size(); q2++) {
      auto b2 = bd2[q2];
      if (cp6::overlapping(b1, b2)) { pc.push_back(ab_link{q1, q2}); }
    }
  }
  return true;
};

inline bool update_pcols_self(const auto &bd, std::vector<ab_link> &pc) {
  for (unsigned q1 = 0; q1 < bd.size(); q1++) {
    auto b1 = bd[q1];
    for (unsigned q2 = q1 + 1; q2 < bd.size(); q2++) {
      auto b2 = bd[q2];
      if (cp6::overlapping(b1, b2)) { pc.push_back(ab_link{q1, q2}); }
    }
  }
  return true;
};

// Basic SAP broadphase
inline bool update_pcols_self(const dlm::vec_ds<aabbf> &bd, sap_pcols &pc) {
  pc.sorted_to_unsorted.clear();
  pc.sorted_to_unsorted.resize(bd.size());
  for (unsigned i = 0; i < bd.size(); i++) { pc.sorted_to_unsorted[i] = i; }

  std::sort(
    pc.sorted_to_unsorted.begin(),
    pc.sorted_to_unsorted.end(),
    [&](unsigned i1, unsigned i2) { return bd[i1].min.x < bd[i2].min.x; }
  );

  for (unsigned i = 0; i < pc.sorted_to_unsorted.size() - 1; i++) {
    unsigned j = i + 1;

    unsigned q = pc.sorted_to_unsorted[i];
    unsigned next_q = pc.sorted_to_unsorted[j];

    while (j < pc.sorted_to_unsorted.size() && bd[q].max.x > bd[next_q].min.x) {
      next_q = pc.sorted_to_unsorted[j];
      if (aabbf::overlapping(bd[q], bd[next_q])) {
        if (q < next_q) {
          pc.push_back({q, next_q});
        } else {
          pc.push_back({next_q, q});
        }
      }
      j++;
    }
  }

  return true;
};

// } // namespace cp6
