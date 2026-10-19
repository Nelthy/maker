#include "basic_types.h"
#include "ikey.h"
#include "merge_keys.h"
#include "print_functions.h"
#include "tests.h"

int main() {
  std::cout << "tests" << std::endl;
  ckey_test();
  ikey_test();
  index_set_test();
  perm_arr_test();
  perm_map_test();
  perm_tuple_test();
  set_remove_queue_ds_test();
  shift_set_remove_queue_test();
  vec_ds_test();

  imap_test();
  dmap_test();
  multi_dmap_test();

  pointer_relay_test();
  reinsert_test();

  merge_keys_test();
  merge_keys_wrap_test();

  sync_ds_test();
  return 0;
}
