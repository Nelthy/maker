#pragma once
#include "ds_modifier.h"
#include "ikey.h"
#include <set>

namespace dlm {
// a null clearer that uses std::set and a pop_back strategey to remove null indices.
template<typename PERMUTE>
class set_remove_queue: public PERMUTE {
protected:
  friend class ds_modifier;

  void clear_null() {
    // clearing communative as long as null isn't called while clearing.
    // this stops infinite recursion
    if (clearing) {
      return;
    }
    clearing = true;

    PERMUTE::clear_null();
    for (const auto &key : m_remove_queue) {
      assert(is_key_valid(*this, key));
      ikey<PERMUTE::rank + 1> back_key = key;
      back_key.end() = ds_modifier::size(*this, back_key.cut_end()) - 1;
      if (key != back_key) {
        PERMUTE::move(key, back_key);
      }
      PERMUTE::resize(back_key.end(), back_key.cut_end());
    }
    m_remove_queue.clear();
    clearing = false;
  }

  template<rank_t KEY_RANK>
  void null(const ikey<KEY_RANK> &key) {
    assert(!clearing);
    if constexpr (KEY_RANK == PERMUTE::rank + 1) {
      if (m_remove_queue.emplace(key).second) {
        PERMUTE::null(key);
      };

    } else {
      PERMUTE::null(key);
    }
  }

  std::set<ikey<PERMUTE::rank + 1>, std::greater<ikey<PERMUTE::rank + 1>>> m_remove_queue;
  bool clearing = false;
};

template<typename T>
using srq = set_remove_queue<T>;
} // namespace dlm
