#pragma once
#include "ds_modifier.h"
#include <vector>
#include "size_ds.h"

struct basic_q_to_cnt: dlm::size_ds {
  inline unsigned q_cnt_num(unsigned q) const {
    assert(q < m_q_num);
    return m_adj[q + 1] - m_adj[q];
  }

  inline const unsigned *get_q_to_cnt(unsigned q) const {
    return &m_adj.data()[m_adj[q]];
  };

  // step 1
  inline void init_storage(unsigned q_num, unsigned cnt_num) {
    m_q_num = q_num;
    m_cnt_num = cnt_num;
    m_temp1.clear();
    m_adj.clear();
    m_temp1.resize(q_num + cnt_num);
    m_adj.resize(q_num + 1 + cnt_num);
    m_adj[q_num] = q_num + 1 + cnt_num;
  };

  // step 2. Loop through all cnts
  inline void register_cnt(unsigned q, unsigned cnt) {
    assert(q < m_q_num);
    assert(cnt < m_cnt_num);
    m_temp1[m_q_num + cnt] = m_temp1[q]++;
  };

  // step 3
  inline void create_q_to_cnt() {
    unsigned sum = m_q_num + 1;
    for (unsigned q = 0; q < m_q_num; q++) {
      m_adj[q] = sum;
      sum += m_temp1[q];
    }
  }

  // step 4. Loop through all cnts
  inline void proj_cnt(unsigned q, unsigned cnt) {
    assert(q < m_q_num);
    assert(cnt < m_cnt_num);

    m_adj[m_adj[q] + m_temp1[m_q_num + cnt]] = cnt;
  }

  // q: [q1_cnt_num,...,qn_cnt_num] cnt: [q1_cnt_num at register...]
  std::vector<unsigned> m_temp1;
  // q: [to_cnt_start...] store: [cnt...]
  std::vector<unsigned> m_adj;
  unsigned m_q_num;
  unsigned m_cnt_num;
};
