#pragma once
#include "basic_types.h"
#include "data_set_tuple.h"
#include "ds_modifier.h"
#include "ds_utils.h"
#include "ikey.h"
#include "perm.h"
#include "perm_map.h"
#include "vec_bundle_ds.h"
#include "vec_ds.h"
#include "vec_ds_alloc.h"

namespace dlm {

// NEW_ALLOC is the allocator for new
// CHANGED_ALLOC is the allocator for changed
// A data set that keeps track of the changes that it undergoes
template<
  typename NEW_ALLOC = std::allocator<index_t>,
  typename CHANGED_ALLOC = std::allocator<index_t>
>
class sync_ds {

  struct i_set: dst<vec_ds<dlm::ikey<1>>, vec_ds<bool>> {
    DEFINE_DS_FIELD((0), inv);
    DEFINE_DS_FIELD((1), is_new);
  };

  struct new_set: perm<vec_ds_alloc<index_t, NEW_ALLOC>, vec_bundle_ds<dlm::perm_map<1, 0>>> {
    DEFINE_DS_FIELD((0), to_i);
    DEFINE_DS_FIELD((1), map);
  };

public:
  static constexpr rank_t rank = 1;

  index_t size() const { return m_i_ds.size(); }

  void clear_sync_data() {
    m_i_ds.inv().assign(m_i_ds.size(), (index_t)-1);
    m_i_ds.is_new().assign(m_i_ds.size(), false);
    m_changed_to_i.clear();
    m_changed_to_start.clear();
    ds_modifier::resize(m_new, 0);
  }

  const unsigned *changed_to_i() const { return m_changed_to_i.data(); }

  const unsigned *changed_to_start() const { return m_changed_to_start.data(); }

  index_t changed_num() const { return m_changed_to_i.size(); }

  const index_t *new_to_i() const { return m_new.to_i().data(); }

  index_t new_num() const { return m_new.size(); }

  template<typename T>
  void add_external_new(T &perm) {
    m_new.map().add_ds(dlm::perm_map<1>{perm});
  }

private:
  i_set m_i_ds;

  new_set m_new;

  std::vector<index_t, CHANGED_ALLOC> m_changed_to_start;
  std::vector<index_t, CHANGED_ALLOC> m_changed_to_i;

  friend class ds_modifier;

  void swap(sync_ds &other, index_t a, index_t b) {
    if (&other != this) {
      this->null(a);
      other.null(b);

      this->m_i_ds.inv()[a] = this->m_new.size();
      this->m_i_ds.is_new()[a] = true;
      grow(this->m_new, 1);
      this->m_new.to_i().back() = a;

      other.m_i_ds.inv()[a] = other.m_new.size();
      other.m_i_ds.is_new()[a] = true;
      grow(other.m_new, 1);
      other.m_new.to_i().back() = a;

    } else {

      // set inverse or add
      {
        index_t inv = m_i_ds.inv()[a];
        if (inv != (index_t)-1) {
          if (m_i_ds.is_new()[a]) {
            m_new.to_i()[inv] = b;
          } else {
            m_changed_to_i[inv] = b;
          }
        } else {
          // add to changed set (if it was new j wouldn't equal -1)
          m_changed_to_start.push_back(a);
          m_changed_to_i.push_back(b);
          m_i_ds.inv()[a] = m_changed_to_i.size() - 1;
        }
      }

      ds_modifier::swap(m_i_ds, ikey{a}, ikey{b});
      // set inverse or add
      {
        index_t inv = m_i_ds.inv()[a];
        if (inv != (index_t)-1) {
          if (m_i_ds.is_new()[a]) {
            m_new.to_i()[inv] = a;
          } else {
            m_changed_to_i[inv] = a;
          }
        } else {
          // add to changed set (if it was new j wouldn't equal -1)
          m_changed_to_start.push_back(b);
          m_changed_to_i.push_back(a);
          m_i_ds.inv()[a] = m_changed_to_i.size() - 1;
        }
      }
    }
  }

  void move(sync_ds &other, index_t dst, index_t src) {
    assert(this->m_i_ds.inv()[dst] == (index_t)-1);
    if (&other != this) {
      other.null(src);

      this->m_i_ds.inv()[dst] = this->m_new.size();
      this->m_i_ds.is_new()[dst] = true;

      grow(this->m_new, 1);
      this->m_new.to_i().back() = dst;

    } else {
      // set inverse or add
      {
        index_t inv = m_i_ds.inv()[src];
        if (inv != (index_t)-1) {
          if (m_i_ds.is_new()[src]) {
            m_new.to_i()[inv] = dst;
          } else {
            m_changed_to_i[inv] = dst;
          }
        } else {
          // add to changed set (if it was new j wouldn't equal -1)
          m_changed_to_start.push_back(src);
          m_changed_to_i.push_back(dst);
          m_i_ds.inv()[src] = m_changed_to_i.size() - 1;
        }
      }
      ds_modifier::move(m_i_ds, ikey{dst}, ikey{src});
    }
  }

  void copy(const sync_ds &other, index_t dst, index_t src) {
    assert(this->m_i_ds.inv()[dst] == (index_t)-1);

    this->m_i_ds.inv()[dst] = this->m_new.size();
    this->m_i_ds.is_new()[dst] = true;
    grow(this->m_new, 1);
    this->m_new.to_i().back() = dst;
  }

  void null(index_t i) {
    index_t inv = m_i_ds.inv()[i];
    if (inv != (index_t)-1) {
      if (m_i_ds.is_new()[i]) {
        // m_new[inv] = m_new.back();
        // m_new.pop_back();
        ds_modifier::move(m_new, ikey{inv}, ikey{m_new.size() - 1});
        ds_modifier::resize(m_new, m_new.size() - 1);

        if (inv != m_new.size()) { m_i_ds.inv()[m_new.to_i()[inv]] = inv; }
      } else {
        m_changed_to_i[inv] = m_changed_to_i.back();
        m_changed_to_start[inv] = m_changed_to_start.back();
        m_changed_to_i.pop_back();
        m_changed_to_start.pop_back();
        if (inv != m_changed_to_i.size()) { m_i_ds.inv()[m_changed_to_i[inv]] = inv; }
      }
      m_i_ds.inv()[i] = (index_t)-1;
    }
  }

  void resize(index_t new_size) {
    index_t old_size = m_i_ds.size();
    ds_modifier::resize(m_i_ds, new_size);
    if (old_size < new_size) {
      for (index_t i = old_size; i < new_size; i++) {
        m_i_ds.inv()[i] = m_new.size();
        m_i_ds.is_new()[i] = true;
        grow(m_new, 1);
        m_new.to_i().back() = i;
      }
    }
  }
};
} // namespace dlm

/*
namespace dlm{
// ALLOC is the allocator used for new and changed
template<typename NEW_ALLOC = std::allocator<dlm::ikey<1>>, typename CHANGED_ALLOC =
std::allocator<ab_link>> class sync_ds {

  struct i_ds: dst<imap<key_vec<1, 1>>, imap<key_vec<1, 1>>> {
    DEFINE_DS_FIELD((0), to_start);
    DEFINE_DS_FIELD((1), to_new);
  };

  struct start_set: perm<dmap<key_vec<1, 1>>, vec_ds<bool>> {
    std::vector<index_t> m_changed_i;

    DEFINE_DS_FIELD((0), to_i);
    DEFINE_DS_FIELD((1), in_changed_i);
  };

  struct new_set:
  srq<perm<dmap<key_vec_alloc<1, NEW_ALLOC>>>> {
    DEFINE_DS_FIELD((0), to_i);
  };

public:
  static constexpr rank_t rank = 1;

  void start_sync_record() {
    if (has_changed) {
      clear(m_start_set);
      clear(m_new_set);
      m_changed.clear();
      m_start_set.m_changed_i.clear();

      grow(m_start_set, m_i_ds.size());
      // relink to identity
      for (index_t i = 0; i < m_i_ds.size(); i++) {
        m_start_set.to_i().link(i, i);
      }
      has_changed = false;
    }
  }

  void end_sync_record() {
    if (has_changed) {
      m_changed.resize(m_start_set.m_changed_i.size());
      for (index_t c = 0; c < m_start_set.m_changed_i.size(); c++) {
        index_t start = m_start_set.m_changed_i[c];
        index_t i = m_start_set.to_i()[start];
        m_changed[c].a = start;
        m_changed[c].b = i;
      }
    }
  }

  const ab_link *get_changed() const { return m_changed.data(); }

  index_t get_changed_num() const { return m_changed.size(); }

  const index_t *get_new() const { return (index_t *)m_new_set.to_i().get_store().data(); }

  index_t get_new_num() const { return m_new_set.size(); }

  sync_ds() {
    // just having start set for the perm since i_ds is a data set
    m_i_ds.to_start().init(m_start_set, ckey<>{}, m_start_set.to_i());
    m_start_set.to_i().init(m_start_set, ckey<>{}, m_i_ds.to_start());

    m_i_ds.to_new().init(m_new_set, ckey<>{}, m_new_set.to_i());
    m_new_set.to_i().init(m_new_set, ckey<>{}, m_i_ds.to_new());
  }

private:
  i_ds m_i_ds;
  start_set m_start_set;
  new_set m_new_set;
  std::vector<ab_link, CHANGED_ALLOC> m_changed;
  bool has_changed = false;

  friend class ds_modifier;

  void swap(sync_ds &other, index_t a, index_t b) {
    if (&other != this) {
      this->null(a);
      other.null(b);
      this->m_i_ds.to_new().sublink(ikey{a}, ikey{});
      other.m_i_ds.to_new().sublink(ikey{b}, ikey{});
      this->has_changed = true;
      other.has_changed = true;
    } else {
      ds_modifier::swap(m_i_ds, ikey{a}, ikey{b});

      ikey<1> start_a = m_i_ds.to_start().get_inverse(a);
      if (start_a.is_0_valid()) {
        if (!m_start_set.in_changed_i()[start_a]) {
          m_start_set.m_changed_i.push_back(start_a);
        }
      }

      ikey<1> start_b = m_i_ds.to_start().get_inverse(b);
      if (start_b.is_0_valid()) {
        if (!m_start_set.in_changed_i()[start_b]) {
          m_start_set.m_changed_i.push_back(start_b);
        }
      }

      has_changed = true;
    }
  }

  void move(sync_ds &other, index_t dst, index_t src) {
    if (&other != this) {
      assert(!this->m_i_ds.to_start().is_0_valid(dst));
      assert(!this->m_i_ds.to_new().is_0_valid(dst));
      other.null(src);

      this->m_i_ds.to_new().sublink(ikey{dst}, ikey{});

      this->has_changed = true;
      other.has_changed = true;
    } else {
      ds_modifier::move(m_i_ds, ikey{dst}, ikey{src});

      ikey<1> start = m_i_ds.to_start().get_inverse(dst);
      if (start.is_0_valid()) {
        if (!m_start_set.in_changed_i()[start]) {
          m_start_set.m_changed_i.push_back(start);
        }
      }
      has_changed = true;
    }
  }

  void copy(const sync_ds &other, index_t dst, index_t src) {
    assert(!this->m_i_ds.to_start().is_0_valid(dst));
    assert(!this->m_i_ds.to_new().is_0_valid(dst));

    this->m_i_ds.to_new().sublink(ikey{dst}, ikey{});

    this->has_changed = true;
  }

  void null(index_t i) {
    ds_modifier::null(m_i_ds, ikey{i});
    clear_null(m_new_set);
  }

  void resize(index_t new_size) {
    index_t old_size = m_i_ds.size();
    ds_modifier::resize(m_i_ds, new_size);
    for (index_t i = old_size; i < new_size; i++) {
      m_i_ds.to_new().sublink(ikey{i}, ikey{});
    }
    has_changed = true;
  }
};
} // namespace cp5

*/
