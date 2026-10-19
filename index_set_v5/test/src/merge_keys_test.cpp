#include "merge_keys.h"
#include "test_common.h"
#include "tests.h"

void merge_keys_test() {
  LOG_FUNC_NAME();
  {
    static constexpr auto ind = dlm::ckey<0, 3, 1, 2>{};
    dlm::ikey a{0, 3};
    dlm::ikey b{1, 2};
    dlm::ikey<4> ab = merge_keys(a, b, ind);

    dlm::ikey<2> a1 = unmerge_key1<2, 2>(ab, ind);
    dlm::ikey<2> b1 = unmerge_key2<2, 2>(ab, ind);

    test_assert((ab == dlm::ikey{0, 1, 2, 3}));
    test_assert(a1 == a);
    test_assert(b1 == b);
  }

  {
    static constexpr auto ind = dlm::ckey<N, 2, 0, 1>{};
    dlm::ikey a{0, 3};
    dlm::ikey b{1, 2};
    dlm::ikey<3> ab = merge_keys(a, b, ind);

    dlm::ikey<2> a1 = unmerge_key1<2, 2>(ab, ind);
    dlm::ikey<2> b1 = unmerge_key2<2, 2>(ab, ind);
    test_assert((ab == dlm::ikey{1, 2, 3}));
    test_assert((a1 == dlm::ikey{-1, 3}));
    test_assert(b1 == b);
  }

  {
    static constexpr auto ind = dlm::ckey<2, 0, 1>{};
    dlm::ikey a{3};
    dlm::ikey b{1, 2};
    dlm::ikey<3> ab = merge_keys(a, b, ind);

    dlm::ikey<1> a1 = unmerge_key1<1, 2>(ab, ind);
    dlm::ikey<2> b1 = unmerge_key2<1, 2>(ab, ind);
    test_assert((ab == dlm::ikey{1, 2, 3}));
    test_assert((a1 == dlm::ikey{3}));
    test_assert(b1 == b);
  }

  {
    static constexpr auto ind = dlm::ckey<0, 1, 2>{};
    dlm::ikey a{0, 1};
    dlm::ikey b{2};
    dlm::ikey<3> ab = merge_keys(a, b, ind);

    dlm::ikey<2> a1 = unmerge_key1<2, 1>(ab, ind);
    dlm::ikey<1> b1 = unmerge_key2<2, 1>(ab, ind);
    test_assert((ab == dlm::ikey{0, 1, 2}));
    test_assert((a1 == dlm::ikey{0, 1}));
    test_assert(b1 == b);
  }
}
