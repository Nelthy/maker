#include "tests.h"
#include "basic_types.h"
#include "operations.h"
#include "perm.h"
#include "relay/pointer_relay_ds.h"
#include "test_common.h"
#include "vec_ds.h"

using namespace dlm;

struct test_ds: vec_ds<float> {
  bool resize_called = false;
  bool move_called = false;
  bool swap_called = false;
  bool null_called = false;
  bool clear_null_called = false;
};

inline void on_resize_overload(test_ds &t, index_t new_size, dlm::ikey<0> key) {
  t.resize_called = true;
}

void on_swap_overload(test_ds &t, test_ds &other, dlm::ikey<1>, dlm::ikey<1>) {
  t.swap_called = true;
}

void on_move_overload(test_ds &t, test_ds &other, dlm::ikey<1>, dlm::ikey<1>) {
  t.move_called = true;
}

void on_null_overload(test_ds &t, dlm::ikey<1>) { t.null_called = true; }

void on_clear_null_overload(test_ds &t) { t.clear_null_called = true; }

void pointer_relay_test() {
  LOG_FUNC_NAME();

  perm<test_ds, pointer_relay_ds<test_ds>> is;
  is.get<1>().init(is.get<0>());

  grow(is, 5);
  test_assert(is.get<0>().resize_called);
  swap(is, 0, 1);
  test_assert(is.get<0>().swap_called);
  null(is, 0);
  test_assert(is.get<0>().null_called);
  clear_null(is);
  test_assert(is.get<0>().clear_null_called);
};
