#pragma once
#include "ikey.h"
#include "print_functions.h"
#include <iostream>

namespace dlm {
template<bool TEST_INV = true, typename MAP, typename INV_MAP>
bool inverse_test(
  const MAP &map,
  const INV_MAP &inv_map,
  bool debug_print = false,
  bool flag_non_valid = true
) {
  static constexpr index_t rank = MAP::rank;
  static constexpr index_t inv_rank = MAP::inv_rank;
  bool result = true;
  apply_nested_ds<rank>(map, [&](dlm::ikey<rank> key) {
    dlm::ikey<inv_rank> inv_key = map.get_inverse(key);

    if (!inv_key.is_0_valid()) {
      if (flag_non_valid) {
        if (debug_print) std::cout << key << "->" << inv_key << " is not valid\n";

        result = false;
      }
      return;
    }

    if (inv_map.get_inverse(inv_key) != key) {
      if (debug_print) {
        std::cout << key << "->" << inv_key << "->" << inv_map.get_inverse(inv_key) << "!=" << key
                  << "\n";
      }
      result = false;
    }
  });

  if (debug_print) {
    if (result) {
      std::cout << "Inverse test passed\n";
    } else {
      std::cout << "Inverse test failed\n";
    }
  }
  if constexpr (TEST_INV) {
    return result && inverse_test<false>(inv_map, map, debug_print, flag_non_valid);
  } else {
    return result;
  }
}

template<index_t I, typename MULTI_MAP, typename INV_MAP>
bool multi_map_inverse_test_one_way(
  const MULTI_MAP &multi_map,
  const INV_MAP &inv_map,
  bool debug_print = false,
  bool flag_non_valid = true
) {
  static constexpr index_t rank = MULTI_MAP::rank;
  static constexpr index_t inv_rank = MULTI_MAP::inv_ranks[I];
  bool result = true;
  apply_nested_ds<rank>(multi_map, [&](dlm::ikey<rank> key) {
    dlm::ikey<inv_rank> inv_key = multi_map.template get_inverse<I>(key);

    if (!inv_key.is_0_valid()) {
      if (flag_non_valid) {
        if (debug_print) std::cout << key << "->" << inv_key << " is not valid\n";

        result = false;
      }
      return;
    }

    if (inv_map.get_inverse(inv_key) != key) {
      if (debug_print) {
        std::cout << key << "->" << inv_key << "->" << inv_map.get_inverse(inv_key) << "!=" << key
                  << "\n";
      }
      result = false;
    }
  });

  if (debug_print) {
    if (result) {
      std::cout << "Inverse test passed\n";
    } else {
      std::cout << "Inverse test failed\n";
    }
  }
  return result;
}
} // namespace dlm
