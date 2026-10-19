#include "tests.h"
#include "perm.h"
#include "test_common.h"
#include "vec_ds.h"
#include <perm_arr.h>

using namespace dlm;

void perm_arr_test() {
  LOG_FUNC_NAME();
  perm_arr<5, perm<vec_ds<int, 2>>> is;

  // for (index_t i = 0; i < 5; i++) {
  //   test_assert(is.get(i).size(ikey{}) == 10);
  // }
}
