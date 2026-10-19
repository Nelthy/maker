#pragma once
#include "maths.h"
#include "vec_ds.h"

namespace cp6 {
/*
  Since body coords depend on the type of coordinate used, and not the cd,
  it is better to output world coordinates.
*/

struct contact {
  // world coordinates
  vec2f x1;
  vec2f x2;

  // normal
  vec2f n{0, 0};

  bool is_valid() const { return n != vec2f{0, 0}; }

  void invalidate() { n = vec2f{0, 0}; }
};

struct contact_1p: contact {
  // coordinate indices
  unsigned q1;
  unsigned q2;
};

struct contact_2p {
  unsigned q1;
  unsigned q2;
  contact cnt1;
  contact cnt2;
};

struct contact_1p_set: dlm::vec_ds<contact_1p> {
  inline unsigned get_q1(unsigned cnt) const { return operator[](cnt).q1; };

  inline unsigned get_q2(unsigned cnt) const { return operator[](cnt).q2; };
};

struct contact_2p_set: dlm::vec_ds<contact_2p> {
  inline unsigned get_q1(unsigned cnt) const { return operator[](cnt).q1; };

  inline unsigned get_q2(unsigned cnt) const { return operator[](cnt).q2; };
};
} // namespace cp6
