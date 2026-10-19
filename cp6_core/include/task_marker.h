#pragma once
#include <bitset>

namespace cp6 {
template<unsigned T_NUM>
class task_marker {
public:
  bool done(unsigned t) {
    m_potential_dones[t] = true;
    // assert(!m_dones[t]);
    return m_dones[t];
  }

  void mark_as_done(unsigned t) {
    assert(!m_dones[t]);
    m_dones[t] = true;
  }

  bool all_done() const {
    for (unsigned i = 0; i < T_NUM; i++) {
      if (m_dones[i] != m_potential_dones[i]) {
        return false;
      }
    }
    return true;
  }

  void reset() {
    for (unsigned i = 0; i < T_NUM; i++) {
      m_dones[i] = false;
      m_potential_dones[i] = false;
    }
  }

private:
  std::bitset<T_NUM> m_dones{};
  std::bitset<T_NUM> m_potential_dones{};
};
} // namespace cp6
