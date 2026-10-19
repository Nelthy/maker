#include "index_maps/base_map.h"
#include "index_maps/imap.h"
#include "index_maps/inverse_test.h"
#include "index_maps/key_vec.h"
#include "index_maps/merge_key_wrap.h"
#include "perm.h"
#include "print_functions.h"
#include "test_common.h"
#include "tests.h"

using namespace dlm;

void merge_keys_wrap_test() {
  LOG_FUNC_NAME();

  perm<
    perm<dlm::imap<dlm::merge_key_wrap<key_vec<2, 1>, 0, N, 1>>>,
    perm<dlm::imap<dlm::merge_key_wrap<key_vec<2, 1>, 0, N, 1>>>
  >
    a;

  init_maps(a, a, ckey<0, 0>{}, ckey<1, 0>{});

  grow(a, 1);
  get<0, 0>(a).sublink(dlm::ikey{0}, dlm::ikey{0});
  get<0, 0>(a).sublink(dlm::ikey{0}, dlm::ikey{0});
  get<0, 0>(a).sublink(dlm::ikey{0}, dlm::ikey{0});

  test_assert(inverse_test(get<0, 0>(a), get<1, 0>(a)));
  test_assert(inverse_test(get<1, 0>(a), get<0, 0>(a)));
}
