#pragma once
#include "basic_types.h"
#include "ckey.h"
#include <utility>

namespace dlm {
template<rank_t P, rank_t... Ps, typename T>
auto &get(T &t, ckey<P, Ps...> c = {}) {
  if constexpr (sizeof...(Ps) == 0) {
    return t.template get<P>();
  } else {
    return dlm::get(t.template get<P>(), ckey<Ps...>{});
  }
}

template<typename T>
auto &get(T &t, ckey<> c = {}) {
  return t;
};

template<rank_t P, rank_t... Ps, typename T>
const auto &get(const T &t, ckey<P, Ps...> c = {}) {
  if constexpr (sizeof...(Ps) == 0) {
    if constexpr (requires { t.template get<P>(); }) {
      return t.template get<P>();
    } else {
      assert(false);
      // static_assert(false, "no supported operation");
    }
  } else {
    return dlm::get(t.template get<P>(), ckey<Ps...>{});
  }
}

template<typename T>
const auto &get(const T &t, ckey<> c = {}) {
  return t;
};

template<typename T, typename CKEY>
using get_t = std::decay_t<decltype(get(std::declval<T &>(), CKEY{}))>;

} // namespace dlm
