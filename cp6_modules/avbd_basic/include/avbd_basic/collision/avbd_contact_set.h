#pragma once
#include "maths.h"
#include "vec_ds.h"

namespace cp6 {

struct avbd_contact {
  vec2f x1;
  vec2f x2;
  vec2f n = {0, 0};
  vec2f lambda = {0, 0.001}; // different so contacts don't stick on the first iterations
  vec2f k = {0, 0};
  vec2f C0 = {0, 0};
  bool stick = false;

  bool is_valid() const { return n != vec2f{0, 0}; }

  void invalidate() {
    *this = avbd_contact{};
  }
};

struct avbd_constants {
  float k = 75'000;
  float alpha = 0.70;
  float beta = 100'000;
  float gamma = .999;
  float friction_coef = 0.5;
  float col_thresh = 1;
  float stick_thresh = 0.00;
};

struct avbd_contact_1p: avbd_contact {
  unsigned q1;
  unsigned q2;
};

struct avbd_contact_2p {
  unsigned q1;
  unsigned q2;
  avbd_contact cnt1;
  avbd_contact cnt2;
};

struct avbd_contact_1p_set: dlm::vec_ds<avbd_contact_1p>, avbd_constants {};

struct avbd_contact_2p_set: dlm::vec_ds<avbd_contact_2p>, avbd_constants {};

// struct avbd_contact_1p_gs_set: avbd_contact_1p_set {
//   // contains the data for q -> cnt
//   std::vector<unsigned> adj;
// };
//
// struct avbd_contact_2p_gs_set: avbd_contact_2p_set {
//   std::vector<unsigned> adj;
// };
} // namespace cp6
