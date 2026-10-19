#include "tests.h"
#include "data_set_tuple.h"
#include "ikey.h"
#include "index_maps/base_map.h"
#include "index_maps/inverse_test.h"
#include "index_maps/key_vec.h"
#include "operations.h"
#include "perm.h"
#include "print_functions.h"
#include "random_utils.h"
#include "set_remove_queue.h"
#include "test_common.h"
#include <index_maps/imap.h>

using namespace dlm;

void imap_r1_threeway_test() {
  LOG_FUNC_NAME();
  perm<srq<perm<imap<key_vec<2, 2>>, imap<key_vec<2, 2>>>>> a;
  perm<srq<perm<imap<key_vec<2, 2>>, imap<key_vec<2, 2>>>>> b;
  perm<srq<perm<imap<key_vec<2, 2>>, imap<key_vec<2, 2>>>>> c;

  // a -> b
  get<0, 0>(a).init(a, ckey<0>{}, get<0, 0>(b));
  // b -> a
  get<0, 0>(b).init(b, ckey<0>{}, get<0, 0>(a));
  // c -> b
  get<0, 0>(c).init(c, ckey<0>{}, get<0, 1>(b));
  // b -> c
  get<0, 1>(b).init(b, ckey<0>{}, get<0, 0>(c));
  // a -> c
  get<0, 1>(a).init(a, ckey<0>{}, get<0, 1>(c));
  // c -> a
  get<0, 1>(c).init(c, ckey<0>{}, get<0, 1>(a));

  static constexpr index_t num = 1000;
  grow(a, num);
  grow(b, num);
  grow(c, num);
  for (index_t i = 0; i < num; i++) {
    get<0, 0>(a).sublink(dlm::ikey{i}, dlm::ikey{i});
    get<0, 1>(b).sublink(dlm::ikey{i}, dlm::ikey{i});
    get<0, 1>(c).sublink(dlm::ikey{i}, dlm::ikey{i});
  }

  for (index_t i = 0; i < num; i++) {
    index_t rand1 = random_index(0, num - 1);
    index_t rand2 = random_index(0, num - 1);
    get<0, 0>(a).sublink(dlm::ikey{rand1}, dlm::ikey{rand2});
    get<0, 1>(b).sublink(dlm::ikey{rand1}, dlm::ikey{rand2});
    get<0, 1>(c).sublink(dlm::ikey{rand1}, dlm::ikey{rand2});
  }

  for (index_t i = 0; i < 100; i++) {
    swap(a, random_index(0, num - 1), random_index(0, num - 1));
    swap(b, random_index(0, num - 1), random_index(0, num - 1));
    swap(c, random_index(0, num - 1), random_index(0, num - 1));
  }
  for (index_t i = 0; i < num; i++) {
    index_t rand1 = random_index(0, num - 1);
    index_t rand2 = random_index(0, num - 1);
    swap<0>(
      a,
      {rand1, random_index(0, size<0>(a, rand1) - 1)},
      {rand2, random_index(0, size<0>(a, rand2) - 1)}
    );
    rand1 = random_index(0, num - 1);
    rand2 = random_index(0, num - 1);
    swap<0>(
      b,
      {rand1, random_index(0, size<0>(b, rand1) - 1)},
      {rand2, random_index(0, size<0>(b, rand2) - 1)}
    );
    rand1 = random_index(0, num - 1);
    rand2 = random_index(0, num - 1);
    swap<0>(
      c,
      {rand1, random_index(0, size<0>(c, rand1) - 1)},
      {rand2, random_index(0, size<0>(c, rand2) - 1)}
    );
  }

  for (index_t i = 0; i < num / 2; i++) {
    index_t rand1 = random_index(0, num - 1);
    null<0>(a, {rand1, random_index(0, size<0>(a, rand1) - 1)});
    rand1 = random_index(0, num - 1);
    null<0>(b, {rand1, random_index(0, size<0>(b, rand1) - 1)});
    rand1 = random_index(0, num - 1);
    null<0>(c, {rand1, random_index(0, size<0>(c, rand1) - 1)});
  }
  clear_null(a);

  test_assert(inverse_test(get<0, 0>(a), get<0, 0>(b), false, false)); // ab
  test_assert(inverse_test(get<0, 0>(b), get<0, 0>(a), false, false)); // ba
  test_assert(inverse_test(get<0, 1>(a), get<0, 1>(c), false, false)); // ac
  test_assert(inverse_test(get<0, 1>(c), get<0, 1>(a), false, false)); // ca
  test_assert(inverse_test(get<0, 1>(b), get<0, 0>(c), false, false)); // bc
  test_assert(inverse_test(get<0, 0>(c), get<0, 1>(b), false, false)); // cb
}

void imap_r1_self_test() {
  LOG_FUNC_NAME();
  srq<perm<imap<key_vec<1, 1>>>> v;
  init_maps(v, v, ckey<0>{}, ckey<0>{});
  grow(v, 4);

  auto reset = [&] {
    for (index_t i = 0; i < 4; i++) {
      get<0>(v).unlink(i);
    }
    get<0>(v).link(0, 2);
    get<0>(v).link(1, 3);
  };

  reset();
  test_assert(get<0>(v)[0] == ikey{2});
  test_assert(get<0>(v)[2] == ikey{0});
  test_assert(get<0>(v)[1] == ikey{3});
  test_assert(get<0>(v)[3] == ikey{1});

  test_assert(inverse_test(get<0>(v), get<0>(v)));

  swap(v, 0, 2);
  test_assert(get<0>(v)[0] == ikey{2});
  test_assert(get<0>(v)[2] == ikey{0});
  swap(v, 0, 3);
  test_assert(inverse_test(get<0>(v), get<0>(v)));
  swap(v, 1, 3);
  test_assert(inverse_test(get<0>(v), get<0>(v)));

  reset();
  null(v, 0);
  test_assert(get<0>(v)[0] == ikey{-1});
  test_assert(get<0>(v)[2] == ikey{-1});
  clear_null(v);
  test_assert(v.size() == 3);
  test_assert(inverse_test(get<0>(v), get<0>(v), false, false));
}

void imap_test() {
  LOG_FUNC_NAME();
  imap_r1_threeway_test();
  imap_r1_self_test();
};
