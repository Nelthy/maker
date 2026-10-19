#include "tests.h"
#include "index_maps/dmap.h"
#include "index_maps/inverse_test.h"
#include "index_maps/key_vec.h"
#include "index_maps/multi_dmap.h"
#include "perm.h"
#include "set_remove_queue.h"
#include "test_common.h"
#include "vec_ds.h"

struct test_multi_store: dlm::vec_ds<std::tuple<dlm::ikey<1>, dlm::ikey<2>>> {
  static constexpr index_t map_num = 2;
  static constexpr rank_t inv_ranks[] = {1, 2};

  template<index_t I>
  auto get_inverse(unsigned i) const {
    return get<I>(this->operator[](i));
  }

  template<index_t I>
  auto set_inverse(unsigned i, dlm::ikey<inv_ranks[I]> value) {
    return get<I>(this->operator[](i)) = value;
  }

  template<index_t I>
  bool is_0_valid(unsigned i) const {
    return get<I>(this->operator[](i)).is_0_valid();
  }
};

void multi_dmap_test() {
  LOG_FUNC_NAME();
  dlm::srq<dlm::perm<dlm::multi_dmap<test_multi_store>>> m;
  dlm::srq<dlm::perm<dlm::dmap<dlm::key_vec<1, 1>>>> m0_inv;
  dlm::srq<dlm::perm<dlm::srq<dlm::perm<dlm::dmap<dlm::key_vec<2, 1>>>>>> m1_inv;
  
  // m1 -> m1_inv
  get<0>(m).init<0>(m, dlm::ckey<>{}, get<0>(m0_inv));
  // m2 -> m2_inv
  get<0>(m).init<1>(m, dlm::ckey<>{}, get<0, 0>(m1_inv));
  // m1_inv -> m1
  get<0>(m0_inv).init(m0_inv, dlm::ckey<>{}, get<0>(m).get_interface<0>());
  // m2_inv -> m2
  get<0, 0>(m1_inv).init(m1_inv, dlm::ckey<0>{}, get<0>(m).get_interface<1>());

  grow(m, 3);
  grow(m0_inv, 3);
  grow(m1_inv, 2);
  grow<0>(m1_inv, 1, 0);
  grow<0>(m1_inv, 2, 1);

  get<0>(m).link<0>(0, 0);
  get<0>(m).link<0>(1, 1);
  get<0>(m).link<0>(2, 2);

  get<0>(m).link<1>(0, {0, 0});
  get<0>(m).link<1>(1, {1, 0});
  get<0>(m).link<1>(2, {1, 1});

  test_assert(dlm::multi_map_inverse_test_one_way<0>(get<0>(m), get<0>(m0_inv)));
  test_assert(dlm::multi_map_inverse_test_one_way<1>(get<0>(m), get<0, 0>(m1_inv)));
  swap(m, 0, 2);
  test_assert(dlm::multi_map_inverse_test_one_way<0>(get<0>(m), get<0>(m0_inv)));
  test_assert(dlm::multi_map_inverse_test_one_way<1>(get<0>(m), get<0, 0>(m1_inv)));
  swap(m, 1, 0);
  test_assert(dlm::multi_map_inverse_test_one_way<0>(get<0>(m), get<0>(m0_inv)));
  test_assert(dlm::multi_map_inverse_test_one_way<1>(get<0>(m), get<0, 0>(m1_inv)));
  swap(m0_inv, 2, 0);
  swap(m1_inv, 0, 1);
  test_assert(dlm::multi_map_inverse_test_one_way<0>(get<0>(m), get<0>(m0_inv)));
  test_assert(dlm::multi_map_inverse_test_one_way<1>(get<0>(m), get<0, 0>(m1_inv)));
  null(m, 0);
  clear_null(m);
  // std::cout << "m0: " << "\n";
  // print_map(get<0>(m0_inv));
  // std::cout << "m1: " << "\n";
  // print_map(get<0, 0>(m1_inv));

  test_assert(dlm::multi_map_inverse_test_one_way<0>(get<0>(m), get<0>(m0_inv)));
  test_assert(dlm::multi_map_inverse_test_one_way<1>(get<0>(m), get<0, 0>(m1_inv)));
}
