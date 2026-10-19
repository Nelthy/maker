#pragma once
#include "basic_types.h"
#include "ckey.h"
#include "ds_utils.h"
#include <iostream>

template<rank_t DEPTH>
std::ostream &operator<<(std::ostream &os, const dlm::ikey<DEPTH> &k) {
  if constexpr (DEPTH > 0) {
    os << "{";
    for (index_t i = 0; i < DEPTH - 1; ++i) {
      if (k[i] != -1) {
        os << k[i] << ", ";
      } else {
        os << "n" << ", ";
      }
    }
    if (k[DEPTH - 1] != -1) {
      os << k[DEPTH - 1] << "}";
    } else {
      os << "n" << "}";
    }
    return os;
  } else {
    os << "{}" << "\n";
    return os;
  }
}

template<rank_t DEPTH = 0>
std::ostream &operator<<(std::ostream &os, const dlm::ikey<0> &k) {
  os << "{}";

  return os;
}

template<rank_t P, rank_t... Ps>
void ckey_print_helper(std::ostream &os, dlm::ckey<P, Ps...> c) {
  if constexpr (sizeof...(Ps) == 0) {
    os << (int)P;
  } else {
    os << (int)P << ", ";
    ckey_print_helper(os, dlm::ckey<Ps...>{});
  }
};

template<rank_t P, rank_t... Ps>
std::ostream &operator<<(std::ostream &os, const dlm::ckey<P, Ps...> &c) {
  os << "<";
  ckey_print_helper(os, c);
  os << ">";
  return os;
}

template<typename DS, rank_t CURR_DEPTH = 0>
void print_shadow(DS &ds, dlm::ikey<CURR_DEPTH> key = {}) {

  if constexpr (CURR_DEPTH == DS::rank - 1) {
    return;
  } else {
    for (rank_t i = 0; i < CURR_DEPTH; i++) {
      std::cout << " ";
    }
    std::cout << key << ": {";

    for (index_t i = 0; i < dlm::ds_modifier::size(ds, key); i++) {
      std::cout << dlm::ds_modifier::size(ds, key.add_end(i));
      if (i != dlm::ds_modifier::size(ds, key) - 1) {
        std::cout << ",";
      }
    }
    std::cout << "}" << std::endl;
    for (index_t i = 0; i < dlm::ds_modifier::size(ds, key); i++) {
      print_shadow(ds, key.add_end(i));
    }
  }
}

template<typename DS>
void simple_print_elements(const DS &ds) {
  dlm::ikey<DS::rank - 1> last_parent;
  if constexpr (DS::rank > 1) {
    for (rank_t i = 0; i < DS::rank - 1; i++) {
      last_parent[i] = 0;
    }
  }

  dlm::apply_nested_ds<DS::rank>(ds, [&](dlm::ikey<DS::rank> key) {
    if (last_parent != key.cut_end()) {
      std::cout << std::endl;
      last_parent = key.cut_end();
    }

    std::cout << get_element(ds, key) << std::endl;
  });
}

template<typename DS>
void print_map(const DS &ds) {
  dlm::ikey<DS::rank - 1> last_parent;
  if constexpr (DS::rank > 1) {
    for (rank_t i = 0; i < DS::rank - 1; i++) {
      last_parent[i] = 0;
    }
  }

  dlm::apply_nested_ds<DS::rank>(ds, [&](dlm::ikey<DS::rank> key) {
    if (last_parent != key.cut_end()) {
      std::cout << std::endl;
      last_parent = key.cut_end();
    }

    std::cout << key <<"->"<<ds.get_inverse(key) << std::endl;
  });
}
