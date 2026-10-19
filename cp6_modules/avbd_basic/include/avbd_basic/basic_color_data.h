#pragma once

// #include <iostream>
struct basic_color_data {
  const unsigned *to_q;
  unsigned cq_num;
};

inline void update_color_data(
  basic_color_data &color_data,
  const unsigned int *to_q,
  unsigned int cq_num
) {

  color_data.to_q = to_q;
  color_data.cq_num = cq_num;
}
