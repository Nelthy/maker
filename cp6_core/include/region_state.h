#pragma once
#include "god_state.h"
#include "perm.h"
#include "perm_map.h"
#include "set_remove_queue.h"
#include "tuple_utils.h"
#include "vec_bundle_ds.h"
#include <comp_ds.h>
#include <external_perm.h>
#include <tuple>
#include <vec_ds.h>

namespace cp6 {
// Stores the coordinate types in regions
// [reg]<qt>[q]
template<typename... QTs>
  requires(all_types_unique_v<QTs...>)
struct region_state:
  dlm::srq<dlm::perm< // [reg]
    dlm::srq<dlm::perm< // <qt>[q]
      dlm::comp_ds<dlm::vec_ds<QTs>>,
      dlm::vec_bundle_ds<dlm::perm_map<2>>,
      dlm::vec_bundle_ds<dlm::perm_map<2, 1>>,
      dlm::vec_bundle_ds<dlm::perm_map_merge<2, 1, 0, 2>>
    >>...,
    dlm::vec_bundle_ds<dlm::perm_map<1>>,
    dlm::vec_bundle_ds<dlm::perm_map<1, 1>>
  >>
{
  static constexpr bool is_region_state = true;
  static constexpr index_t qt_num = sizeof...(QTs);
  using qt_ts = std::tuple<QTs...>;

  template<index_t QT>
  auto &get_qt(index_t reg) {
    static_assert(QT < sizeof...(QTs));
    return get<QT, 0>(*this)[reg];
  }

  template<index_t QT>
  const auto &get_qt(index_t reg) const {
    static_assert(QT < sizeof...(QTs));
    return get<QT, 0>(*this)[reg];
  }

  // template<typename QT>
  // auto &get_qt(index_t reg) {
  //   static_assert(
  //     filter_indices<std::is_same, qt_ts>().size() == 1,
  //     "must have exactly one qt matching type"
  //   );
  //   return get_qt<filter_indices<same_as<QT>::template cond, qt_ts>()[0]>(reg);
  // }
  //
  // template<typename QT>
  // const auto &get_qt(index_t reg) const {
  //   static_assert(
  //     filter_indices<std::is_same, qt_ts>().size() == 1,
  //     "must have exactly one qt matching type"
  //   );
  //   return get_qt<filter_indices<std::is_same, qt_ts>()[0]>(reg);
  // }

  template<typename IS>
  void add_r1_external(IS &is) {
    this->template get<qt_num>().add_ds(dlm::perm_map<1>{is});
  }

  template<typename IS, rank_t P>
  void add_r11_external(IS &is, dlm::ckey<P> is_perm, unsigned rt_key) {
    this->template get<qt_num + 1>().add_ds(dlm::perm_map<1, 1>{is, is_perm, rt_key});
  }

  template<index_t QT, typename IS, rank_t P>
  void add_r2_external(IS &is, dlm::ckey<P> is_perm) {
    get<QT, 1>(*this).add_ds(dlm::perm_map<2>{is, is_perm});
  }

  template<index_t QT, typename IS, rank_t P1, rank_t P2>
  void add_r21_external(IS &is, dlm::ckey<P1, P2> is_perm, unsigned rt_key) {
    get<QT, 2>(*this).add_ds(dlm::perm_map<2, 1>{is, is_perm, rt_key});
  }

  template<index_t QT, typename IS, rank_t P1, rank_t P2>
  void add_merge_external(IS &is, dlm::ckey<P1, P2> is_perm, unsigned rt_key) {
    get<QT, 3>(*this).add_ds(dlm::perm_map_merge<2, 1, 0, 2>(is, is_perm, rt_key));
  }
};

template<typename T>
struct is_region_state {
  static constexpr bool value = requires { T::is_region_state; };
};

template<typename TW>
using region_state_t = get_t<is_region_state, typename TW::types>;

template<typename TW>
constexpr unsigned get_qt_num() {
  return region_state_t<TW>::qt_num;
}

template<typename TW>
using qt_ts = typename region_state_t<TW>::qt_ts;

template<index_t QT, typename TW>
using qt_t = std::tuple_element_t<QT, qt_ts<TW>>;

template<index_t QT, typename TW>
using cd_t = typename std::decay_t<decltype(std::declval<qt_t<QT, TW>>().cd())>;

template<index_t QT, typename TW>
using qd_t = typename std::decay_t<decltype(std::declval<qt_t<QT, TW>>().qd())>;

template<typename T, typename TW>
constexpr unsigned get_qt_index() {
  static_assert(filter_tuple<same_as<T>::template cond, qt_ts<TW>>().size() == 1);
  return filter_tuple<same_as<T>::template cond, qt_ts<TW>>()[0];
}

} // namespace cp6
