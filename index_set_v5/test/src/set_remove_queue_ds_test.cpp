#include "tests.h"
#include "operations.h"
#include "perm.h"
#include "set_remove_queue.h"
#include "test_common.h"
#include "vec_ds.h"

using namespace dlm;

void set_remove_queue_ds_r1_test() {
  LOG_FUNC_NAME();

  srq<perm<vec_ds<int>>> is;
  grow(is, 10);
  for (size_t i = 0; i < is.size(); i++) {
    is.get<0>()[i] = i;
  }

  null(is, 0);
  null(is, 1);
  null(is, 2);
  null(is, 2);
  // is.swap(0, 9); // throws
  // is.grow(3); // doesn't throw
  clear_null(is);
  test_assert(is.size() == 7);
  test_assert(is.get<0>()[0] == 7);
  test_assert(is.get<0>()[1] == 8);
  test_assert(is.get<0>()[2] == 9);
  test_assert(is.get<0>()[3] == 3);
  test_assert(is.get<0>()[4] == 4);
  test_assert(is.get<0>()[5] == 5);
  test_assert(is.get<0>()[6] == 6);
}

void set_remove_queue_ds_r2_test() {
  LOG_FUNC_NAME();
  {
    srq<perm<srq<perm<vec_ds<ikey<2>, 2>>>>> is;
    grow(is, 3);
    grow<0>(is, 1, {0});
    grow<0>(is, 2, {1});
    grow<0>(is, 3, {2});

    for (size_t i = 0; i < is.size(); i++) {
      for (size_t j = 0; j < size<0>(is, {i}); j++) {
        get<0, 0>(is)[i][j] = ikey{i, j};
      }
    }

    null<0>(is, {0, 0});
    null<0>(is, {1, 0});
    null<0>(is, {2, 0});
    null<0>(is, {2, 1});
    clear_null(is);

    test_assert((size<0>(is, {0}) == 0));
    test_assert((size<0>(is, {1}) == 1));
    test_assert((size<0>(is, {2}) == 1));

    test_assert((get<0, 0>(is)[1][0] == ikey{1, 1}));
    test_assert((get<0, 0>(is)[2][0] == ikey{2, 2}));
  }

  {
    srq<perm<srq<perm<vec_ds<ikey<2>, 2>>>>> is;
    grow(is, 3);
    grow<0>(is, 10, {0});
    grow<0>(is, 20, {1});
    grow<0>(is, 3, {2});

    for (size_t i = 0; i < is.size(); i++) {
      for (size_t j = 0; j < size<0>(is, {i}); j++) {
        get<0, 0>(is)[i][j] = ikey{i, j};
      }
    }

    null(is, 0);
    null(is, 1);
    null<0>(is, {2, 0});
    null<0>(is, {2, 1});

    clear_null(is);

    test_assert(is.size() == 1);
    test_assert(size<0>(is, {0}) == 1);
    test_assert((get<0, 0>(is)[0][0] == ikey{2, 2}));
  }
}

void set_remove_queue_ds_test() {
  LOG_FUNC_NAME();
  set_remove_queue_ds_r1_test();
  set_remove_queue_ds_r2_test();
};
