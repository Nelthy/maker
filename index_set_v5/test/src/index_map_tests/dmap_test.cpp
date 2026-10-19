#include "data_set_tuple.h"
#include "index_maps/base_map.h"
#include "index_maps/dmap.h"
#include "index_maps/imap.h"
#include "index_maps/inverse_test.h"
#include "index_maps/key_vec.h"
#include "operations.h"
#include "perm.h"
#include "print_functions.h"
#include "set_remove_queue.h"
#include "test_common.h"
#include "tests.h"

using namespace dlm;

void dmap_r1_to_r2_self_map_test() {
  LOG_FUNC_NAME();
  srq<perm<imap<key_vec<1, 2>>, srq<perm<dmap<key_vec<2, 1>>>>>> v;
  static constexpr ckey<0> a;
  static constexpr ckey<1, 0> b;

  init_maps(v, v, a, b);

  grow(v, 4);
  get(v, b).sublink(ikey{3}, ikey{0});
  get(v, b).sublink(ikey{3}, ikey{1});
  get(v, b).sublink(ikey{3}, ikey{2});
  get(v, b).sublink(ikey{0}, ikey{3});
  

  null(v, 0);
  clear_null(v);
  test_assert(inverse_test(get(v, a), get(v, b), false, false));
  dlm::clear(v, 2, b.cut_end());
  test_assert(inverse_test(get(v, a), get(v, b), false, false));

}

void dmap_test() {
  LOG_FUNC_NAME();
  dmap_r1_to_r2_self_map_test();
};
