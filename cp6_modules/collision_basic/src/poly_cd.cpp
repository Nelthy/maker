#include "collision_basic/poly_cd.h"
#include "maths.h"

void cp6::poly_cd::recenter_geometry(unsigned q) {
  vec2f centroid = base_cd()[q].calculate_centroid();
  if (centroid != vec2f{0, 0}) {
    for (unsigned i = 0; i < base_cd()[q].vertices.size(); i++) {
      base_cd()[q].vertices[i] -= centroid;
    }
  }
}

void cp6::poly_cd::calculate_constants(unsigned q, float *area, float *area_inertia) const {
  if (area != nullptr) {
    *area = base_cd()[q].calculate_area();
  }
  if (area_inertia != nullptr) {
    *area_inertia = base_cd()[q].calculate_area_inertia();
  }
}

unsigned cp6::poly_cd::vertex_num(unsigned q) const {
  if (q != size() - 1) {
    return to_transformed_cd()[q + 1] - to_transformed_cd()[q];
  } else {
    return m_transformed_cd.size() - to_transformed_cd()[q];
  }
}

void cd_transform_start(cp6::poly_cd &cd) {
  // copying vertices to transformed collider data
  // performance could be improved if the base vertices where cached
  cd.m_transformed_cd.clear();
  for (unsigned q = 0; q < cd.size(); q++) {
    cd.to_transformed_cd()[q] = cd.m_transformed_cd.size();
    cd.m_transformed_cd.insert(
      cd.m_transformed_cd.end(),
      cd.base_cd()[q].vertices.begin(),
      cd.base_cd()[q].vertices.end()
    );
  }
}

void cd_transform_end(cp6::poly_cd &cd) {
  for (unsigned q = 0; q < cd.size(); q++) {
    unsigned vertex_num = cd.vertex_num(q);
    if (vertex_num > 0) {
      vec2f *q_vertices = cd.get_vertices(q);
      cd.bounds()[q].min = q_vertices[0];
      cd.bounds()[q].max = q_vertices[0];

      for (unsigned q_v = 1; q_v < cd.vertex_num(q); q_v++) {
        cd.bounds()[q].encompass(q_vertices[q_v]);
      }
    } else {
      cd.bounds()[q].invalidiate();
    }
  }
}

void cd_translate(cp6::poly_cd &cd, const vec2f *translation) {
  // transforming each vertex
  unsigned cursor = 0;
  for (unsigned c = 0; c < cd.size(); c++) {
    for (unsigned c_v = 0; c_v < cd.vertex_num(c); c_v++) {
      cd.m_transformed_cd[cursor] += translation[c];
      cursor++;
    }
  }
}

void cd_rotate(cp6::poly_cd &cd, const float *rotation) {
  unsigned cursor = 0;
  for (unsigned c = 0; c < cd.size(); c++) {
    for (unsigned c_v = 0; c_v < cd.vertex_num(c); c_v++) {
      cd.m_transformed_cd[cursor] = rotate(cd.m_transformed_cd[cursor], rotation[c]);
      cursor++;
    }
  }
}
