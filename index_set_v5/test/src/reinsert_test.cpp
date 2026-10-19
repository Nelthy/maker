#include "ds_modifier.h"
#include "ikey.h"
#include "print_ds.h"
#include "print_functions.h"
#include "test_common.h"
#include "tests.h"
#include "vec_ds.h"

using namespace dlm;


void r1_test() {
  LOG_FUNC_NAME();
  dlm::vec_ds<unsigned> v;
  ds_modifier::resize(v, 3);
  v[0] = 2;
  v[1] = 0;
  v[2] = 1;
  ds_modifier::reinsert(v, ikey{}, 2, 0);
  test_assert(v[0] == 0);
  test_assert(v[1] == 1);
  test_assert(v[2] == 2);
  v[0] = 1;
  v[1] = 0;
  v[2] = 2;
  ds_modifier::reinsert(v, ikey{}, 0, 2);
  test_assert(v[0] == 2);
  test_assert(v[1] == 1);
  test_assert(v[2] == 0);
}

void r2_test() {
  LOG_FUNC_NAME();
  dlm::vec_ds<unsigned, 2> v;
  ds_modifier::resize(v, 2);
  ds_modifier::resize(v, 3, ikey{1});
  v[1][0] = 2;
  v[1][1] = 0;
  v[1][2] = 1;
  ds_modifier::reinsert(v, ikey{1}, 2, 0);
  test_assert(v[1][0] == 0);
  test_assert(v[1][1] == 1);
  test_assert(v[1][2] == 2);
  v[1][0] = 1;
  v[1][1] = 0;
  v[1][2] = 2;
  ds_modifier::reinsert(v, ikey{1}, 0, 2);
  test_assert(v[1][0] == 2);
  test_assert(v[1][1] == 1);
  test_assert(v[1][2] == 0);
}


void reinsert_test() {
  LOG_FUNC_NAME();
  r1_test();
  r2_test();
}
