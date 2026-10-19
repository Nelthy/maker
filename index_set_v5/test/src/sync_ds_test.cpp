#include "comp_ds.h"
#include "operations.h"
#include "perm.h"
#include "set_remove_queue.h"
#include "sync_ds.h"
#include "test_common.h"
#include "tests.h"

struct r1_test_sync_set: dlm::srq<dlm::perm<dlm::vec_ds<float>, dlm::sync_ds<>>> {
  DEFINE_DS_FIELD((0), floats);
  DEFINE_DS_FIELD((1), sync);
};

static void r1_test() {
  LOG_FUNC_NAME();
  r1_test_sync_set s;
  dlm::grow(s, 10);
  dlm::swap(s, 0, 9);
  test_assert(s.sync().new_num() == 10);
  test_assert(s.sync().new_to_i()[0] == 9);
  test_assert(s.sync().new_to_i()[1] == 1);
  test_assert(s.sync().new_to_i()[2] == 2);
  test_assert(s.sync().new_to_i()[3] == 3);
  test_assert(s.sync().new_to_i()[4] == 4);
  test_assert(s.sync().new_to_i()[5] == 5);
  test_assert(s.sync().new_to_i()[6] == 6);
  test_assert(s.sync().new_to_i()[7] == 7);
  test_assert(s.sync().new_to_i()[8] == 8);
  test_assert(s.sync().new_to_i()[9] == 0);

  s.sync().clear_sync_data();
  dlm::grow(s, 2);
  dlm::swap(s, 0, 3);
  dlm::swap(s, 10, 11);
  dlm::swap(s, 5, 10);
  test_assert(s.sync().new_num() == 2);
  test_assert(s.sync().new_to_i()[0] == 11);
  test_assert(s.sync().new_to_i()[1] == 5);

  test_assert(s.sync().changed_num() == 3);
  // test_assert((s.sync().changed()[0] == dlm::start_to_i{0, 3}));
  // test_assert((s.sync().changed()[1] == dlm::start_to_i{3, 0}));
  // test_assert((s.sync().changed()[2] == dlm::start_to_i{5, 10}));
  test_assert((s.sync().changed_to_start()[0] == 0));
  test_assert((s.sync().changed_to_start()[1] == 3));
  test_assert((s.sync().changed_to_start()[2] == 5));

  test_assert((s.sync().changed_to_i()[0] == 3));
  test_assert((s.sync().changed_to_i()[1] == 0));
  test_assert((s.sync().changed_to_i()[2] == 10));

  // for (unsigned i = 0; i < s.sync().changed_num(); i++) {
  //   std::cout<<s.sync().changed()[i].start << "->" << s.sync().get_changed()[i].i<<"\n";
  // }

  s.sync().clear_sync_data();
  dlm::null(s, 5);
  dlm::clear_null(s);

  test_assert(s.sync().new_num() == 0);
  test_assert(s.sync().changed_num() == 1);
  test_assert((s.sync().changed_to_start()[0] == 11));
  test_assert((s.sync().changed_to_i()[0] ==  5));
}

struct r2_test_sync_set:
dlm::perm<dlm::perm<dlm::vec_ds<float, 2>, dlm::comp_ds<dlm::vec_ds<dlm::sync_ds<>>>>> {
  DEFINE_DS_FIELD((0, 0), floats);
  DEFINE_DS_FIELD((0, 1), sync);
};

static void r2_test() {
  LOG_FUNC_NAME();
  r2_test_sync_set s;
  dlm::grow(s, 2);
  dlm::grow<0>(s, 2, {0});
  dlm::grow<0>(s, 3, {1});

  test_assert((s.sync()[0].new_num() == 2));
  test_assert(s.sync()[0].new_to_i()[0] == 0);
  test_assert(s.sync()[0].new_to_i()[1] == 1);

  test_assert((s.sync()[1].new_num() == 3));
  test_assert(s.sync()[1].new_to_i()[0] == 0);
  test_assert(s.sync()[1].new_to_i()[1] == 1);
  test_assert(s.sync()[1].new_to_i()[2] == 2);

  dlm::swap(s, 0, 1);

  test_assert((s.sync()[0].new_num() == 3));
  test_assert((s.sync()[1].new_num() == 2));

  s.sync()[0].clear_sync_data();
  s.sync()[1].clear_sync_data();

  dlm::swap<0>(s, {0, 0}, {1, 0});
  dlm::grow<0>(s, 1, {1});
  test_assert((s.sync()[0].new_num() == 1));
  test_assert((s.sync()[1].new_num() == 2));
}

void sync_ds_test() {
  LOG_FUNC_NAME();
  r1_test();
  r2_test();
}
