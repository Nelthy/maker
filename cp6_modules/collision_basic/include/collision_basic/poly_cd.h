#pragma once
#include "aabb.h"
#include "data_set_tuple.h"
#include "ds_utils.h"
#include "vec_ds.h"
#include "vector_poly.h"
// user defined function
template<typename QT>
vec2f transform_point(unsigned q, const QT &transform_state, const vec2f &point);

namespace cp6 {
struct poly_cd: dlm::dst<dlm::vec_ds<vector_poly>, dlm::vec_ds<unsigned>, dlm::vec_ds<aabbf>> {
  DEFINE_DS_FIELD((0), base_cd);
  DEFINE_DS_FIELD((1), to_transformed_cd);
  DEFINE_DS_FIELD((2), bounds); 

  poly_cd &cd() { return *this; }

  const poly_cd &cd() const { return *this; }

  const dlm::vec_ds<aabbf> &bd() const { return bounds(); }

  // helper functions
  void calculate_constants(unsigned c, float *area, float *area_inertia) const;
  unsigned vertex_num(unsigned c) const;

  const vec2f *get_vertices(unsigned c) const {
    return &m_transformed_cd.data()[to_transformed_cd()[c]];
  }

  vec2f *get_vertices(unsigned c) { return &m_transformed_cd[to_transformed_cd()[c]]; }

  void recenter_geometry(unsigned c);

  std::vector<vec2f> m_transformed_cd;
};
} // namespace cp6

void cd_transform_start(cp6::poly_cd &cd);
void cd_transform_end(cp6::poly_cd &cd);

// helper transform template
template<typename QT>
void cd_transform(cp6::poly_cd &cd, const QT &transform_state) {
  // transforming each vertex
  unsigned cursor = 0;
  for (unsigned c = 0; c < cd.size(); c++) {
    for (unsigned c_v = 0; c_v < cd.vertex_num(c); c_v++) {
      cd.m_transformed_cd[cursor] =
        transform_point(c, transform_state, cd.m_transformed_cd[cursor]);
      cursor++;
    }
  }
}

template<typename QT>
void cd_ind_transform(cp6::poly_cd &cd, const QT &transform_state, const unsigned *ind) {
  // transforming each vertex
  unsigned cursor = 0;
  for (unsigned c = 0; c < cd.size(); c++) {
    unsigned qt_c = ind[c];
    for (unsigned c_v = 0; c_v < cd.vertex_num(c); c_v++) {
      cd.m_transformed_cd[cursor] =
        transform_point(qt_c, transform_state, cd.m_transformed_cd[cursor]);
      cursor++;
    }
  }
}
