#pragma once
#include "basic_types.h"
#include "ikey.h"
#include <cstddef>

namespace dlm {
template<rank_t... Ps>
struct ckey {
  template<rank_t... _Ps>
  static constexpr auto add_end(ckey<_Ps...> = {}) {
    return ckey<Ps..., _Ps...>{};
  }

  template<rank_t... _Ps>
  static constexpr auto add_beg(ckey<_Ps...> = {}) {
    return ckey<_Ps..., Ps...>{};
  }

  template<rank_t Np = 1>
  static constexpr auto cut_end() {
    static_assert(sizeof...(Ps) > 0);
    if constexpr (sizeof...(Ps) > 0) {
      return []<size_t... Is>(std::index_sequence<Is...>) {
        constexpr rank_t arr[] = {Ps...};
        return ckey<arr[Is]...>{};
      }(std::make_index_sequence<sizeof...(Ps) - Np>{});
    }
  }

  template<rank_t Np = 1>
  static constexpr auto cut_beg() {
    return []<size_t... Is>(std::index_sequence<Is...>) {
      constexpr rank_t arr[] = {Ps...};
      return ckey<arr[Np + Is]...>{};
    }(std::make_index_sequence<sizeof...(Ps) - Np>{});
  }

  static auto get_ikey() { return ikey{Ps...}; }

  template<rank_t... _Ps>
  static constexpr bool eql_to(ckey<_Ps...> other) {
    if constexpr (sizeof...(Ps) == sizeof...(_Ps)) {
      return ((Ps == _Ps) && ...);
    } else {
      return false;
    }
  }
};
} // namespace dlm
