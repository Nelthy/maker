#include "tests.h"
#include "operations.h"
#include "perm.h"
#include "print_ds.h"
#include "print_functions.h"
#include "test_common.h"
#include "vec_ds.h"
#include <iostream>

using namespace dlm;

void basic_r1_operation_test() {
  LOG_FUNC_NAME();
  {
    perm<vec_ds<float>> v;
    grow(v, 10);
    auto reset = [&] {
      for (size_t i = 0; i < v.size(); i++) {
        get<0>(v)[i] = i;
      }
    };
    reset();
    test_assert(v.size() == 10);

    swap(v, 0, 9);
    test_assert(get<0>(v)[0] == 9);
    test_assert(get<0>(v)[9] == 0);
    reset();

    swap(v, ikey{0}, ikey{9});
    test_assert(get<0>(v)[0] == 9);
    test_assert(get<0>(v)[9] == 0);
    reset();

    null(v, 0);
  }

  {
    perm<vec_ds<std::vector<float>>> v;
    grow(v, 10);
    auto reset = [&] {
      for (size_t i = 0; i < v.size(); i++) {
        get<0>(v)[i].resize(i);
      }
    };
    reset();

    shift_right(v, 5, 3);
    test_assert(v.size() == 13);
    test_assert(get<0>(v)[5].size() == 0);
    test_assert(get<0>(v)[6].size() == 0);
    test_assert(get<0>(v)[7].size() == 0);
    test_assert(get<0>(v)[8].size() == 5);
    test_assert(get<0>(v)[9].size() == 6);
    test_assert(get<0>(v)[10].size() == 7);
    test_assert(get<0>(v)[11].size() == 8);
    test_assert(get<0>(v)[12].size() == 9);
  }

  // {
  //   perm<vec_ds<std::vector<float>>> v1;
  //   perm<vec_ds<std::vector<float>>> v2;
  //   v1.grow(3);
  //   v2.grow(2);
  //   for (size_t i = 0; i < v1.size(); i++) {
  //     vget<0>(1)[i].resize(i);
  //   }
  //   for (size_t i = 0; i < v2.size(); i++) {
  //     vget<0>(2)[i].resize(i + 10);
  //   }
  //
  //   v1.insert(v2, 0, 0, v2.size());
  //   test_assert(v1.size() == 5);
  //   test_assert(vget<0>(1)[0].size() == 10);
  //   test_assert(vget<0>(1)[1].size() == 11);
  //   test_assert(vget<0>(1)[2].size() == 0);
  //   test_assert(vget<0>(1)[3].size() == 1);
  //   test_assert(vget<0>(1)[4].size() == 2);
  //
  //   test_assert(vget<0>(2)[0].size() == 0);
  //   test_assert(vget<0>(2)[1].size() == 0);
  // }

  // {
  //   perm<vec_ds<std::vector<float>>> v1;
  //   perm<vec_ds<std::vector<float>>> v2;
  //   v1.grow(3);
  //   v2.grow(2);
  //   for (size_t i = 0; i < v1.size(); i++) {
  //     vget<0>(1)[i].resize(i);
  //   }
  //   for (size_t i = 0; i < v2.size(); i++) {
  //     vget<0>(2)[i].resize(i + 10);
  //   }
  //
  //   v1.insert(v2, v1.size(), 0, v2.size());
  //   test_assert(v1.size() == 5);
  //   test_assert(vget<0>(1)[0].size() == 0);
  //   test_assert(vget<0>(1)[1].size() == 1);
  //   test_assert(vget<0>(1)[2].size() == 2);
  //   test_assert(vget<0>(1)[3].size() == 10);
  //   test_assert(vget<0>(1)[4].size() == 11);
  //
  //   test_assert(vget<0>(2)[0].size() == 0);
  //   test_assert(vget<0>(2)[1].size() == 0);
  // }

  // {
  //   perm<vec_ds<std::vector<float>>> v1;
  //   perm<vec_ds<std::vector<float>>> v2;
  //   v1.grow(3);
  //   v2.grow(2);
  //   for (size_t i = 0; i < v1.size(); i++) {
  //     vget<0>(1)[i].resize(i);
  //   }
  //   for (size_t i = 0; i < v2.size(); i++) {
  //     vget<0>(2)[i].resize(i + 10);
  //   }
  //
  //   v1.insert(v2, 1, 0, 1);
  //   test_assert(v1.size() == 4);
  //   test_assert(vget<0>(1)[0].size() == 0);
  //   test_assert(vget<0>(1)[1].size() == 10);
  //   test_assert(vget<0>(1)[2].size() == 1);
  //   test_assert(vget<0>(1)[3].size() == 2);
  //
  //   test_assert(vget<0>(2)[0].size() == 0);
  //   test_assert(vget<0>(2)[1].size() == 11);
  // }
}

void basic_r2_operation_test() {
  LOG_FUNC_NAME();
  {
    perm<perm<vec_ds<ikey<2>, 2>>> v;
    grow(v, 3);
    grow<0>(v, 1, {0});
    grow<0>(v, 2, {1});
    grow<0>(v, 3, {2});

    test_assert((v.size() == 3));
    test_assert((size<0>(v, {0}) == 1));
    test_assert((size<0>(v, {1}) == 2));
    test_assert((size<0>(v, {2}) == 3));

    auto reset = [&] {
      for (size_t i = 0; i < v.size(); i++) {
        for (size_t j = 0; j < size<0>(v, i); j++) {
          get<0, 0>(v)[i][j] = ikey{i, j};
        }
      }
    };
    reset();

    swap<0>(v, {1, 0}, {1, 1});
    test_assert((get<0, 0>(v)[1][0] == ikey{1, 1}));
    test_assert((get<0, 0>(v)[1][1] == ikey{1, 0}));
    reset();

    swap<0>(v, {0, 0}, {1, 1});
    test_assert((get<0, 0>(v)[0][0] == ikey{1, 1}));
    test_assert((get<0, 0>(v)[1][1] == ikey{0, 0}));
    reset();

    null(v, 0);
    null<0>(v, {0, 0});
    reset();
  }

  {
    perm<perm<vec_ds<ikey<2>, 2>>, perm<vec_ds<ikey<2>, 2>>> v;
    grow(v, 3);
    grow<0>(v, 1, {0});
    grow<0>(v, 2, {1});
    grow<0>(v, 3, {2});

    grow<1>(v, 4, {0});
    grow<1>(v, 5, {1});
    grow<1>(v, 6, {2});

    test_assert(size<0>(v, {0}) == 1);
    test_assert(size<0>(v, {1}) == 2);
    test_assert(size<0>(v, {2}) == 3);

    test_assert(size<1>(v, {0}) == 4);
    test_assert(size<1>(v, {1}) == 5);
    test_assert(size<1>(v, {2}) == 6);

    merge(v, 0, 2);
    test_assert(size<0>(v, {0}) == 4);
    test_assert(size<0>(v, {2}) == 0);
    test_assert(size<1>(v, {0}) == 10);
    test_assert(size<1>(v, {2}) == 0);
  }
}

void index_set_test() {
  LOG_FUNC_NAME();
  basic_r1_operation_test();
  basic_r2_operation_test();
}
