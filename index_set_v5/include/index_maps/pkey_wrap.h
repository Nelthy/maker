#pragma once
#include "ckey.h"

namespace dlm {
// Compile time key insert
template<
  rank_t START_RANK,
  rank_t END_RANK = 0,
  rank_t INSERT_KEY_RANK,
  rank_t CURR_RANK = 0,
  rank_t INSERTED_NUM = 0,
  rank_t INSERT_RANK = 0,
  rank_t INSERT_INDIRECT = 0,
  rank_t... INSERT
>
void ct_key_insert(
  const ikey<START_RANK> &start_key,
  ckey<INSERT_RANK, INSERT_INDIRECT, INSERT...> insert_data,
  const ikey<INSERT_KEY_RANK> &insert_key,
  ikey<END_RANK> &end_key,
  ckey<CURR_RANK> = {},
  ckey<INSERTED_NUM> = {}
) {
  static_assert(sizeof...(INSERT) % 2 == 0);
  static_assert(START_RANK < END_RANK);

  if constexpr (CURR_RANK < END_RANK) {
    static constexpr rank_t insert_index = rank_t(INSERT_RANK + INSERTED_NUM);
    if constexpr (CURR_RANK == insert_index) {
      if constexpr (sizeof...(INSERT) > 0) {
        ct_key_insert(
          start_key,
          ckey<INSERT...>{},
          insert_key,
          end_key,
          ckey<CURR_RANK + 1>{},
          ckey<INSERTED_NUM + 1>{}
        );
      } else {
        // no more inserts, so make up some values
        ct_key_insert(
          start_key,
          ckey<(rank_t)-1, (rank_t)-1>{},
          insert_key,
          end_key,
          ckey<CURR_RANK + 1>{},
          ckey<INSERTED_NUM + 1>{}
        );
      }
      end_key[insert_index] = insert_key[INSERT_INDIRECT];
    } else {
      ct_key_insert(
        start_key,
        ckey<INSERT_RANK, INSERT_INDIRECT, INSERT...>{},
        insert_key,
        end_key,
        ckey<CURR_RANK + 1>{},
        ckey<INSERTED_NUM>{}
      );
      end_key[CURR_RANK] = start_key[CURR_RANK - INSERTED_NUM];
    }
  }
}

template<
  rank_t START_RANK,
  rank_t END_RANK = 0,
  rank_t CURR_RANK = 0,
  rank_t EXTRACTED_NUM = 0,
  rank_t INSERT_RANK = 0,
  rank_t INSERT_INDIRECT = 0,
  rank_t... INSERT
>
void ct_key_extract(
  const ikey<START_RANK> &start_key,
  ckey<INSERT_RANK, INSERT_INDIRECT, INSERT...> insert_data,
  ikey<END_RANK> &end_key,
  ckey<CURR_RANK> = {},
  ckey<EXTRACTED_NUM> = {}
) {
  static_assert(sizeof...(INSERT) % 2 == 0);
  static_assert(START_RANK > END_RANK);

  if constexpr (CURR_RANK < START_RANK) {
    if constexpr (CURR_RANK - EXTRACTED_NUM == INSERT_RANK) {
      // no nothing, and go to the next one
      if constexpr (sizeof...(INSERT)) {
        // go to next insert
        ct_key_extract(
          start_key,
          ckey<INSERT...>(),
          end_key,
          ckey<CURR_RANK + 1>(),
          ckey<EXTRACTED_NUM + 1>()
        );
      } else {
        // make up some values because there are no more inserts
        ct_key_extract(
          start_key,
          ckey<(rank_t)-1, (rank_t)-1>(),
          end_key,
          ckey<CURR_RANK + 1>(),
          ckey<EXTRACTED_NUM + 1>()
        );
      }

    } else {
      end_key[CURR_RANK - EXTRACTED_NUM] = start_key[CURR_RANK];
      ct_key_extract(
        start_key,
        ckey<INSERT_RANK, INSERT_INDIRECT>(),
        end_key,
        ckey<CURR_RANK + 1>(),
        ckey<EXTRACTED_NUM>()
      );
    }
  }
}

// Partial key wrapper. There are cases where the inverse has an index that is always the same
// as an index in the key used to get it. We can use this fact to only store the necessary keys.
// This takes in a storage type and produces a new storage type.
// INV_INSERT:
//   must be a set of number PAIRS
//   The first number describes the rank of the output and the second is the rank of the input.
//   The insert function inserts the rank of the input into the rank of the output.
//   This is the pattern:
//     inv_rank, input_rank, inv_rank, input rank
template<typename STORE_T, rank_t... INV_INSERT>
class pkey_wrap: public STORE_T {
  static_assert(sizeof...(INV_INSERT) % 2 == 0);

public:
  static constexpr index_t rank = STORE_T::rank;
  static constexpr index_t inv_rank = STORE_T::inv_rank + sizeof...(INV_INSERT) / 2;

  ikey<inv_rank> get_inverse(ikey<rank> key) const {
    assert(key.is_valid());
    auto compress_key = STORE_T::get_inverse(key);
    return uncompress_key(key, compress_key);
  }

  void set_inverse(ikey<rank> key, ikey<inv_rank> value) {
    assert(key.is_valid());
    ikey<STORE_T::inv_rank> compress_key;
    ct_key_extract(value, ckey<INV_INSERT...>{}, compress_key);
    STORE_T::set_inverse(key, compress_key);
  }

  static inline ikey<inv_rank> uncompress_key(
    ikey<STORE_T::rank> key,
    ikey<STORE_T::inv_rank> compressed_inv_key
  ) {
    if (!compressed_inv_key.is_0_valid()) {
      // for performance reasons, the base map only checks the key[0] for validity
      // so when partial key replace 0 with a valid value every time we run into problems.
      // To fix, check if the compress's 0 is valid and if its not,
      // return an key with a invalid 0.
      return ikey<inv_rank>{}; // fully null key
    } else {
      ikey<inv_rank> full_key;
      ct_key_insert(compressed_inv_key, ckey<INV_INSERT...>{}, key, full_key);
      return full_key;
    }
  }
};
} // namespace dlm

/*
  The first number describes the rank of the output and the first is the rank of the input.
  The insert function inserts the rank of the input into the rank of the output.

  inv_rank, input_rank, inv_rank, input rank

  ikey<4> end_key;
  ct_key_insert(ikey{0, 1}, ckey<0, 0, 1, 1>{}, ikey{69, 420}, end_key);
  std::cout << "expected: {69, 0, 420, 1}\n";
  std::cout << "got: " << end_key << std::endl;

  ct_key_insert(ikey{0, 1, 0}, ckey<2, 0>{}, ikey{420}, end_key);
  std::cout << "expected: {0, 1, 0, 420}\n";
  std::cout << "got: " << end_key << std::endl;

  ct_key_insert(ikey{2, 3, 4}, ckey<0, 0>{}, ikey{1}, end_key);
  std::cout << "expected: {1, 2, 3, 4}\n";
  std::cout << "got: " << end_key << std::endl;
*/
