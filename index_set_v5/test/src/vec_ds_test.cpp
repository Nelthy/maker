#include "tests.h"
#include "ds_modifier.h"
#include "ds_utils.h"
#include "ikey.h"
#include "vec_ds.h"
#include <print_functions.h>
#include <test_common.h>
using namespace dlm;

// testing r1 operations work on vec_ds
void vec_ds_r1_test() {
  LOG_FUNC_NAME();
  {
    vec_ds<float> v;
    auto reset = [&] {
      ds_modifier::resize(v, 10, ikey{});
      for (size_t i = 0; i < v.size(); i++) {
        v[i] = i;
      }
    };
    reset();
    test_assert(v.size() == 10);

    ds_modifier::move(v, ikey{0}, ikey{9});
    test_assert(v[0] == 9);
    test_assert(v[9] == 9);
    reset();

    ds_modifier::move(v, v, ikey{0}, ikey{9});
    test_assert(v[0] == 9);
    test_assert(v[9] == 9);
    reset();

    ds_modifier::swap(v, ikey{0}, ikey{9});
    test_assert(v[0] == 9);
    test_assert(v[9] == 0);
    reset();

    ds_modifier::swap(v, v, ikey{0}, ikey{9});
    test_assert(v[0] == 9);
    test_assert(v[9] == 0);
    reset();

    ds_modifier::copy(v, v, ikey{0}, ikey{9});
    test_assert(v[0] == 9);
    test_assert(v[9] == 9);
  }
  {
    vec_ds<std::vector<float>> v;
    ds_modifier::resize(v, 10, ikey{});
    auto reset = [&] {
      for (size_t i = 0; i < v.size(); i++) {
        v[i].resize(i);
      }
    };
    reset();
    ds_modifier::copy(v, v, ikey{0}, ikey{9});
    test_assert(v[0].size() == 9);
    test_assert(v[9].size() == 9);
  }
}

void vec_ds_r2_test() {
  LOG_FUNC_NAME();
  {
    vec_ds<int, 2> v;
    ds_modifier::resize(v, 3, ikey{});

    ds_modifier::resize(v, 1, ikey{0});
    ds_modifier::resize(v, 2, ikey{1});
    ds_modifier::resize(v, 3, ikey{2});

    test_assert(is_key_valid(v, ikey{0, 0}));
    test_assert(!is_key_valid(v, ikey{0, 1}));

    test_assert(is_key_valid(v, ikey{1, 0}));
    test_assert(is_key_valid(v, ikey{1, 1}));
    test_assert(!is_key_valid(v, ikey{1, 2}));

    test_assert(is_key_valid(v, ikey{2, 0}));
    test_assert(is_key_valid(v, ikey{2, 1}));
    test_assert(is_key_valid(v, ikey{2, 2}));
    test_assert(!is_key_valid(v, ikey{2, 3}));
  }
  {
    vec_ds<ikey<2>, 2> v;
    ds_modifier::resize(v, 3, ikey{});
    ds_modifier::resize(v, 1, ikey{0});
    ds_modifier::resize(v, 2, ikey{1});
    ds_modifier::resize(v, 3, ikey{2});

    for (unsigned i = 0; i < v.size(); i++) {
      for (unsigned j = 0; j < v.size(ikey{i}); j++) {
        v[i][j] = ikey{i, j};
      }
    }
    ds_modifier::move(v, ikey{0, 0}, ikey{1, 0});
    test_assert((v[0][0] == ikey{1, 0}));
    test_assert((v[1][0] == ikey{-1, -1}));

    ds_modifier::move(v, v, ikey{0, 0}, ikey{1, 1});
    test_assert((v[0][0] == ikey{1, 1}));
    test_assert((v[1][1] == ikey{-1, -1}));

    for (unsigned i = 0; i < v.size(); i++) {
      for (unsigned j = 0; j < v.size(ikey{i}); j++) {
        v[i][j] = ikey{i, j};
      }
    }
  }

  {
    vec_ds<ikey<2>, 2> v;
    ds_modifier::resize(v, 3, ikey{});

    ds_modifier::resize(v, 1, ikey{0});
    ds_modifier::resize(v, 2, ikey{1});
    ds_modifier::resize(v, 3, ikey{2});

    for (unsigned i = 0; i < v.size(); i++) {
      for (unsigned j = 0; j < v.size(ikey{i}); j++) {
        v[i][j] = ikey{i, j};
      }
    }

    ds_modifier::swap(v, ikey{0, 0}, ikey{1, 0});
    test_assert((v[0][0] == ikey{1, 0}));
    test_assert((v[1][0] == ikey{0, 0}));

    for (unsigned i = 0; i < v.size(); i++) {
      for (unsigned j = 0; j < v.size(ikey{i}); j++) {
        v[i][j] = ikey{i, j};
      }
    }

    ds_modifier::swap(v, v, ikey{0, 0}, ikey{1, 0});
    test_assert((v[0][0] == ikey{1, 0}));
    test_assert((v[1][0] == ikey{0, 0}));
  }

  {
    vec_ds<ikey<2>, 2> v;
    ds_modifier::resize(v, 3, ikey{});

    ds_modifier::resize(v, 1, ikey{0});
    ds_modifier::resize(v, 2, ikey{1});
    ds_modifier::resize(v, 3, ikey{2});

    for (unsigned i = 0; i < v.size(); i++) {
      for (unsigned j = 0; j < v.size(ikey{i}); j++) {
        v[i][j] = ikey{i, j};
      }
    }

    ds_modifier::merge(v, ikey{0}, ikey{2});
    test_assert(v[0].size() == 4);
    test_assert((v[0][0] == ikey{0, 0}));
    test_assert((v[0][1] == ikey{2, 0}));
    test_assert((v[0][2] == ikey{2, 1}));
    test_assert((v[0][3] == ikey{2, 2}));
  }
}

void vec_ds_test() {
  LOG_FUNC_NAME();
  vec_ds_r1_test();
  vec_ds_r2_test();
}
