#include "test_common.h"
#include "tests.h"
#include "ds_utils.h"
#include "ikey.h"
#include "operations.h"
#include "perm.h"
#include "print_functions.h"
#include "shift_set_remove_queue.h"
#include "vec_ds.h"

using namespace dlm;

void shift_set_remove_queue_test() {
  LOG_FUNC_NAME();
  ssrq<perm<ssrq<perm<dlm::vec_ds<dlm::ikey<2>, 2>>>>> p;
  grow(p, 2);
  grow<0>(p, 3, {0});
  grow<0>(p, 4, {1});

  apply_nested_ds<2>(get<0, 0>(p), [&](dlm::ikey<2> key) { get<0, 0>(p)[key] = key; });

  null<0>(p, {0, 0});
  null<0>(p, {1, 0});
  null<0>(p, {1, 2});
  
  clear_null(p);
  test_assert(dlm::size(p) == 2);
  test_assert(dlm::size<0>(p, {0}) == 2)
  test_assert(dlm::size<0>(p, {1}) == 2)
  
  test_assert((get<0,0>(p)[0][0] == ikey{0, 1})) 
  test_assert((get<0,0>(p)[0][1] == ikey{0, 2}))

  test_assert((get<0,0>(p)[1][0] == ikey{1, 1})) 
  test_assert((get<0,0>(p)[1][1] == ikey{1, 3})) 

};
