#include "tests.h"
#include "test_common.h"
#include <ikey.h>
using namespace dlm;

void ikey_test() {
  LOG_FUNC_NAME();
  {
    ikey<2> key;
    test_assert((key == ikey{-1, -1}));
    ikey key1{1, 2};
    ikey key2{3, 4};
    ikey<4> key3 = {key1, key2};
    test_assert((key3 == ikey{1, 2, 3, 4}));
    ikey<5> key4 = {key1, key1, ikey{-1}};
    test_assert((key4 == ikey{1, 2, 1, 2, -1}));
    ikey<3> key5 = key4.cut_end<2>();
    test_assert((key5 == ikey{1, 2, 1}));
    ikey<2> key6 = key5.cut_beg<1>();
    test_assert((key6 == ikey{2, 1}));
    test_assert((key5.cut_beg<2>() == ikey{1}));
  }
  {
    ikey<0> key;
    test_assert(key.add_end(1) == ikey{1});
    test_assert((key.add_end(0).add_end(ikey{1, 2}) == ikey{0, 1, 2}));
    test_assert(key.add_beg(1) == ikey{1});
    test_assert((key.add_beg(0).add_beg(ikey{1, 2}) == ikey{1, 2, 0}));
  }
  {
    ikey key{1, 2, 3, 4, 5};
    test_assert((key.shrink_to_beg<3>() == ikey{1, 2, 3}));
    test_assert((key.shrink_to_end<3>() == ikey{3, 4, 5}));
  }
  // TODO: test comparison operators.
};
