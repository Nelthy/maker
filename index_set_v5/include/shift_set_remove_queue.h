#pragma once
#include "ds_modifier.h"
#include <set>

namespace dlm {
// a null clearer that uses std::set and a shift strategy to remove null indices.
template<typename PERMUTE>
class shift_set_remove_queue: public PERMUTE {
public:
  friend class ds_modifier;

  void clear_null() {
    // clearing commutative as long as null isn't called while clearing.
    // this stops infinite recursion
    if (clearing) {
      return;
    }

    clearing = true;
    PERMUTE::clear_null();
    for (const auto &key : m_remove_queue) {
      assert(is_key_valid(*this, key));

      ikey<PERMUTE::rank> curr_parent_key = key.cut_end();
      for (unsigned i = key.end() + 1; i < ds_modifier::size(*this, curr_parent_key); i++) {
        PERMUTE::move(curr_parent_key.add_end(i - 1), curr_parent_key.add_end(i));
      }
      PERMUTE::resize(ds_modifier::size(*this, curr_parent_key) - 1, curr_parent_key);
    }
    m_remove_queue.clear();
    clearing = false;
  }

  template<rank_t KEY_RANK>
  void null(const ikey<KEY_RANK> &key) {
    assert(!clearing && "cannot null while clearing");
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
using ssrq = shift_set_remove_queue<T>;
} // namespace dlm
