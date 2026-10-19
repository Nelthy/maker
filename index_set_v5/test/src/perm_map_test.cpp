#include "arr_ds.h"
#include "comp_ds.h"
#include "external_perm.h"
#include "operations.h"
#include "perm.h"
#include "perm_arr.h"
#include "perm_map.h"
#include "print_ds.h"
#include "print_functions.h"
#include "set_remove_queue.h"
#include "test_common.h"
#include "tests.h"
#include "vec_ds.h"
#include <cassert>
#include "size_ds.h"

using namespace dlm;

void r3_remove_perm_test() {
  LOG_FUNC_NAME();
  // clang-format off
  srq<perm<
    srq<perm<
      srq<perm<
        vec_ds<float, 3>,
        perm_map<3>
      >>,
      perm_map<2>
    >>,
    perm_map<1>
  >> a;

  ext<perm<
    ext<perm<
      ext<perm<
        vec_ds<float, 3>
      >>
    >>
  >> b;

  // clang-format on
  get<1>(a).init(b, ckey<>{});
  get<0, 1>(a).init(b, ckey<0>{});
  get<0, 0, 1>(a).init(b, ckey<0, 0>{});

  grow(a, 2);
  grow<0>(a, 2, {0});
  grow<0, 0>(a, 3, {0, 0});
  grow<0, 0>(a, 3, {0, 1});
  test_assert((b.size() == 2));
  test_assert((size<0>(b, {0}) == 2));
  test_assert((size<0, 0>(b, {0, 0}) == 3));
  test_assert((size<0, 0>(b, {0, 1}) == 3));
  null(a, {1});
  clear_null(a);
  test_assert((b.size() == 1));

  null<0>(a, {0, 0});
  clear_null(a);

  test_assert((size<0>(b, {0}) == 1));

  null<0, 0>(a, {0, 0, 0});
  null<0, 0>(a, {0, 0, 1});
  clear_null(a);

  test_assert((size<0, 0>(b, {0, 0}) == 1));
}

void rt_key_r3_perm_map_test() {
  LOG_FUNC_NAME();
  perm<perm<dlm::vec_ds<float, 2>, perm_map<2, 2>>, perm_map<1, 2>> a;

  ext<perm<ext<perm<ext<perm<ext<perm<comp_ds<arr_ds<2, arr_ds<2, vec_ds<float, 2>>>>>>>>>>>> b;

  get<1>(a).init(b, ckey<0, 0>{}, ikey{0, 1});
  get<0, 1>(a).init(b, ckey<0, 0, 0>{}, ikey{0, 1});
  grow(a, 5);
  grow<0>(a, 6, {2});
  grow<0>(a, 7, {4});

  bool result = true;

  test_assert((size<0, 0>(b, {0, 1}) == 5));
  test_assert((size<0, 0, 0>(b, {0, 1, 2}) == 6));
  test_assert((size<0, 0, 0>(b, {0, 1, 4}) == 7));
  swap(a, 2, 4);
  test_assert((size<0, 0, 0>(b, {0, 1, 2}) == 7));
  test_assert((size<0, 0, 0>(b, {0, 1, 4}) == 6));
}

void extra_permute_perm_test() {
  LOG_FUNC_NAME();
  perm<size_ds, perm_map<1>> a;
  ext<perm<perm<vec_ds<float, 2>>>> b;
  get<1>(a).init(b);

  grow(a, 1);
  test_assert(b.size() == 1);

  grow<0>(b, 2, {0});
  test_assert(size<0>(b, {0}) == 2);
};

void r1_merge_perm_map_test() {
  LOG_FUNC_NAME();
  perm<size_ds, perm_map<1>> v;
  ext<perm<perm<vec_ds<float, 2>>>> ext_v;
  get<1>(v).init(ext_v);

  grow(v, 3);
  grow<0>(ext_v, 1, {0});
  grow<0>(ext_v, 2, {1});
  grow<0>(ext_v, 3, {2});

  merge(v, 0, 2);
  test_assert(size<0>(ext_v, {0}) == 4);
  test_assert(size<0>(ext_v, {2}) == 0);
}

void perm_map_merge_test() {
  LOG_FUNC_NAME();
  static constexpr rank_t p_num = 3;
  // [a]<p>[b]
  perm_arr<p_num, perm<vec_ds<float, 2>, perm_map_merge<2, 1, 0, 2>>, perm_map<1>> v;
  // [a][p][b]
  ext<
    perm<perm<ext<perm<comp_ds<vec_ds<arr_ds<p_num, vec_ds<ikey<2>>>>>, print_ds<3, "external">>>>>
  >
    ext_v;
  get<0, 0, 1>(ext_v).enabled = false;

  get<3>(v).init(ext_v);

  get<0, 1>(v).init(ext_v, ckey<0, 0>{}, 0);
  get<1, 1>(v).init(ext_v, ckey<0, 0>{}, 1);
  get<2, 1>(v).init(ext_v, ckey<0, 0>{}, 2);

  grow(v, 2);
  test_assert(dlm::size(ext_v) == 2);

  unsigned a = 1;
  grow<0>(v, 1, {a});
  grow<1>(v, 2, {a});
  grow<2>(v, 3, {a});

  test_assert((dlm::size<0, 0>(ext_v, {a, 0}) == 1));
  test_assert((dlm::size<0, 0>(ext_v, {a, 1}) == 2));
  test_assert((dlm::size<0, 0>(ext_v, {a, 2}) == 3));

  for (unsigned p = 0; p < p_num; p++) {
    for (unsigned b = 0; b < dlm::size<0, 0>(ext_v, {a, p}); b++) {
      get<0, 0, 0>(ext_v)[a][p][b] = {a, b};
    }
  }

  swap(v, 0, 1);
  a = 0;
  test_assert((size<0, 0>(ext_v, {a, 0}) == 1));
  test_assert((size<0, 0>(ext_v, {a, 1}) == 2));
  test_assert((size<0, 0>(ext_v, {a, 2}) == 3));

  static constexpr rank_t swap_p = 2;
  test_assert((get<0, 0, 0>(ext_v)[a][swap_p][0] == dlm::ikey{1, 0}));
  test_assert((get<0, 0, 0>(ext_v)[a][swap_p][2] == dlm::ikey{1, 2}));
  swap<swap_p>(v, {a, 0}, {a, 2});
  test_assert((get<0, 0, 0>(ext_v)[a][swap_p][0] == dlm::ikey{1, 2}));
  test_assert((get<0, 0, 0>(ext_v)[a][swap_p][2] == dlm::ikey{1, 0}));
  // works with print_ds. I'm done testin
  null<0>(v, {0, 0});
  clear_null(v);
  clear<0>(v, 0);
}

void r21_perm_map_test() { LOG_FUNC_NAME(); }

void perm_map_test() {
  LOG_FUNC_NAME();
  rt_key_r3_perm_map_test();
  r3_remove_perm_test();
  extra_permute_perm_test();
  r1_merge_perm_map_test();
  r21_perm_map_test();
  perm_map_merge_test();
}
