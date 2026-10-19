#include "tests.h"
#include "print_functions.h"
#include "test_common.h"
#include <sys/types.h>
#include <utility>

using namespace dlm;

void ckey_test() {
  LOG_FUNC_NAME();
  ckey<0, 1, 2, 3> c;
  static_assert(ckey<0, 1, 2, 3>().eql_to(ckey<0, 1, 2, 3>{}));
  static_assert(!ckey<0, 1, 2, 3>().eql_to(ckey<0, 0, 2, 3>{}));
  test_assert((c.cut_end<2>().get_ikey() == ikey{0, 1}));
  test_assert((c.add_beg(ckey<4, 5>{}).get_ikey() == ikey{4, 5, 0, 1, 2, 3}));
  test_assert((c.add_end(ckey<4, 5>{}).get_ikey() == ikey{0, 1, 2, 3, 4, 5}));
  test_assert((c.add_beg(ckey<4, 5>{}).get_ikey() == ikey{4, 5, 0, 1, 2, 3}));
};
