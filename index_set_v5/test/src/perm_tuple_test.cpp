#include "tests.h"
#include "operations.h"
#include "perm.h"
#include "perm_tuple.h"
#include "test_common.h"
#include "vec_ds.h"
#include <array>
using namespace dlm;

template<index_t P>
struct test_generator {
  using type = dlm::perm<dlm::vec_ds<std::array<unsigned, P>, 2>>;
};

void perm_tuple_test() {
  LOG_FUNC_NAME();
  perm_tuple<3, test_generator> is;
  grow(is, 10);
  test_assert(is.get<0>().size() == 10);
  test_assert(is.get<1>().size() == 10);
  test_assert(is.get<2>().size() == 10);
  grow<0>(is, 1, {0});
  grow<1>(is, 2, {0});
  grow<2>(is, 3, {0});
  test_assert(is.get<0>().size(ikey{0}) == 1);
  test_assert(is.get<1>().size(ikey{0}) == 2);
  test_assert(is.get<2>().size(ikey{0}) == 3);
  // the array sizes
  test_assert((get<0, 0>(is)[0][0].size() == 0));
  test_assert((get<0, 0>(is)[0][0].size() == 0));
  test_assert((get<1, 0>(is)[0][0].size() == 1));
  test_assert((get<2, 0>(is)[0][0].size() == 2));
};
