#include "collision_basic/narrowphase_fns.h"
#include "ab_link.h"
#include "ds_modifier.h"
#include "maths.h"

namespace cp6 {
bool narrowphase(
  const circle_cd &cd_a,
  const circle_cd &cd_b,
  const std::vector<ab_link> &pcols,
  contact_1p_set &dst
) {
  dlm::ds_modifier::resize(dst, 1);
  for (unsigned i = 0; i < pcols.size(); i++) {
    cp6::cvc_np(cd_a, cd_b, pcols[i].a, pcols[i].b, dst.back());
    if (dst.back().is_valid()) {
      dst.back().q1 = pcols[i].a;
      dst.back().q2 = pcols[i].b;
      dlm::ds_modifier::resize(dst, dst.size() + 1);
    }
  }

  dlm::ds_modifier::resize(dst, dst.size() - 1);
  return true;
}

bool narrowphase(
  const circle_cd &cd_a,
  const poly_cd &cd_b,
  const std::vector<ab_link> &pcols,
  contact_1p_set &dst
) {
  dlm::ds_modifier::resize(dst, 1);

  for (unsigned i = 0; i < pcols.size(); i++) {
    cp6::cvp_np(cd_a, cd_b, pcols[i].a, pcols[i].b, dst.back());
    if (dst.back().is_valid()) {
      dst.back().q1 = pcols[i].a;
      dst.back().q2 = pcols[i].b;
      dlm::ds_modifier::resize(dst, dst.size() + 1);
    }
  }

  dlm::ds_modifier::resize(dst, dst.size() - 1);
  return true;
}

bool narrowphase(
  const poly_cd &cd_a,
  const poly_cd &cd_b,
  const std::vector<ab_link> &pcols,
  contact_2p_set &dst
) {
  dlm::ds_modifier::resize(dst, 1);
  for (unsigned i = 0; i < pcols.size(); i++) {
    cp6::pvp_np(cd_a, cd_b, pcols[i].a, pcols[i].b, dst.back().cnt1, dst.back().cnt2);
    if (dst.back().cnt1.is_valid()) {
      dst.back().q1 = pcols[i].a;
      dst.back().q2 = pcols[i].b;

      dlm::ds_modifier::resize(dst, dst.size() + 1);
    }
  }

  dlm::ds_modifier::resize(dst, dst.size() - 1);
  return true;
}

void cvc_np(
  const circle_cd &cd1,
  const circle_cd &cd2,
  unsigned c1,
  unsigned c2,
  contact &dst_contact
) {
  circle a = cd1[c1];
  circle b = cd2[c2];
  float dist_squared = length_squared(a.pos - b.pos);
  float radii_sum_squared = (a.radius + b.radius) * (a.radius + b.radius);
  if (dist_squared <= radii_sum_squared) {
    vec2f n = (b.pos - a.pos);
    if (length_squared(n) < 1e-5) {
      n = vec2f{1, 0};
    } else {
      n /= length(n);
    }
    vec2f xa = a.pos + (n * a.radius);
    vec2f xb = b.pos + (-n * b.radius);

    dst_contact = contact{xa, xb, n};
  }
}

static inline bool point_in_convex_polygon(const vec2f *verts, unsigned v_num, const vec2f &p) {
  // returns true if p is inside or on edge of convex polygon (any winding)
  bool has_pos = false;
  bool has_neg = false;
  for (unsigned i = 0; i < v_num; ++i) {
    const vec2f &a = verts[i];
    const vec2f &b = verts[(i + 1) % v_num];
    vec2f edge = b - a;
    vec2f rel = p - a;
    // 2D cross product (scalar)
    float cross = edge.x * rel.y - edge.y * rel.x;
    if (cross > 0.0f)
      has_pos = true;
    else if (cross < 0.0f)
      has_neg = true;
    if (has_pos && has_neg) return false;
  }
  return true;
}

// Idk, I made chatGPT make this shit
void cvp_np(
  const circle_cd &c_cd,
  const poly_cd &p_cd,
  unsigned ci,
  unsigned pi,
  contact &dst_contact
) {
  const circle &c = c_cd[ci];
  unsigned v_num = p_cd.vertex_num(pi);
  const vec2f *verts = p_cd.get_vertices(pi);

  // --- SAT over polygon edge normals ---
  float min_overlap = std::numeric_limits<float>::infinity();
  for (unsigned i = 0; i < v_num; ++i) {
    const vec2f &va = verts[i];
    const vec2f &vb = verts[(i + 1) % v_num];
    vec2f edge = vb - va;
    vec2f axis = vec2f{-edge.y, edge.x}; // perpendicular
    float axis_len_sq = length_squared(axis);
    if (axis_len_sq == 0.0f) continue;
    axis /= length(axis);

    float p_min = dot(verts[0], axis);
    float p_max = p_min;
    for (unsigned j = 1; j < v_num; ++j) {
      float pj = dot(verts[j], axis);
      if (pj < p_min) p_min = pj;
      if (pj > p_max) p_max = pj;
    }

    float c_proj = dot(c.pos, axis);
    float c_min = c_proj - c.radius;
    float c_max = c_proj + c.radius;

    float overlap = std::min(p_max, c_max) - std::max(p_min, c_min);
    if (overlap < 0.0f) {
      return; // separating axis
    }
    if (overlap < min_overlap) min_overlap = overlap;
  }

  // --- also test axis from closest vertex (Voronoi region) ---
  float best_v_dist_sq = std::numeric_limits<float>::infinity();
  int best_v_index = -1;
  for (unsigned i = 0; i < v_num; ++i) {
    float d2 = length_squared(verts[i] - c.pos);
    if (d2 < best_v_dist_sq) {
      best_v_dist_sq = d2;
      best_v_index = int(i);
    }
  }
  if (best_v_index >= 0) {
    vec2f axis_v = (verts[best_v_index] - c.pos);
    float axis_v_len_sq = length_squared(axis_v);
    if (axis_v_len_sq > 0.0f) {
      axis_v /= length(axis_v);
      float p_min = dot(verts[0], axis_v);
      float p_max = p_min;
      for (unsigned j = 1; j < v_num; ++j) {
        float pj = dot(verts[j], axis_v);
        if (pj < p_min) p_min = pj;
        if (pj > p_max) p_max = pj;
      }
      float c_proj = dot(c.pos, axis_v);
      float overlap = std::min(p_max, c_proj + c.radius) - std::max(p_min, c_proj - c.radius);
      if (overlap < 0.0f) return;
      if (overlap < min_overlap) min_overlap = overlap;
    }
  }

  // --- compute closest point on polygon boundary to circle center ---
  float min_dist_sq = std::numeric_limits<float>::infinity();
  vec2f closest_point{};
  int closest_edge = -1;
  for (unsigned i = 0; i < v_num; ++i) {
    const vec2f &a = verts[i];
    const vec2f &b = verts[(i + 1) % v_num];
    vec2f ab = b - a;
    float ab_len_sq = length_squared(ab);
    if (ab_len_sq == 0.0f) {
      float d2 = length_squared(a - c.pos);
      if (d2 < min_dist_sq) {
        min_dist_sq = d2;
        closest_point = a;
        closest_edge = int(i);
      }
      continue;
    }
    float t = dot((c.pos - a), ab) / ab_len_sq;
    if (t < 0.0f)
      t = 0.0f;
    else if (t > 1.0f)
      t = 1.0f;
    vec2f proj = a + ab * t;
    float d2 = length_squared(proj - c.pos);
    if (d2 < min_dist_sq) {
      min_dist_sq = d2;
      closest_point = proj;
      closest_edge = int(i);
    }
  }

  // --- determine if center is inside polygon ---
  bool center_inside = point_in_convex_polygon(verts, v_num, c.pos);

  // If center is not inside and closest distance is greater than radius -> no contact
  if (!center_inside) {
    float radius_sq = c.radius * c.radius;
    if (min_dist_sq > radius_sq) {
      return;
    }
  }

  // --- compute contact normal (points from circle center toward polygon surface) ---
  vec2f n = (closest_point - c.pos);
  float n_len = length(n);
  if (n_len == 0.0f) {
    // degenerate: center exactly on boundary or closest_point equals center
    if (closest_edge >= 0) {
      const vec2f &a = verts[closest_edge];
      const vec2f &b = verts[(closest_edge + 1) % v_num];
      vec2f edge = b - a;
      n = vec2f{-edge.y, edge.x};
      float nl = length(n);
      if (nl == 0.0f)
        n = vec2f{1.0f, 0.0f};
      else
        n = n * (1.0f / nl);
      if (dot(n, closest_point - c.pos) < 0.0f) n = n * -1.0f;
    } else {
      n = vec2f{1.0f, 0.0f};
    }
  } else {
    n = n * (1.0f / n_len);
  }
  if (center_inside) {
    n *= -1.f;
  }

  // contact on circle surface and on polygon boundary
  vec2f contact_on_circle = c.pos + n * c.radius;
  vec2f contact_on_poly = closest_point;

  dst_contact = contact{contact_on_circle, contact_on_poly, n};
}

struct edge {
  vec2f a;
  vec2f b;
};

struct simplex {
  // p, d
  struct simplex_vertex {
    vec2f p; // position
    vec2f d; // direction
  };

  simplex_vertex v[3];
  char count = 0;

  inline void add_vertex(vec2f p, vec2f d) {
    v[2] = v[1];
    v[1] = v[0];
    v[0] = {p, d};
    this->count++;
    assert(this->count <= 3);
  };

  inline void remove_vertex(unsigned char vertex_index) {
    v[vertex_index] = {};
    for (char i = vertex_index; i < 2; i++) {
      v[i] = v[i + 1];
    }
    count--;
    v[2] = {};
  }
};

struct uv {
  float u;
  float v;
};

struct uvw {
  float u;
  float v;
  float w;
};

inline uv compute_uv_coords(const vec2f &a, const vec2f &b, const vec2f &Q) {
  if (b == a) return uv{1, 0};

  vec2f n = b - a;
  float l = length(n);
  n /= l;
  return uv{(dot(b - Q, n)) / l, (dot((Q - a), n)) / l};
}

inline float area(const vec2f &a, const vec2f &b, const vec2f &c) {
  return cross(b - a, c - a) * 0.5;
}

inline uvw compute_uvw_coords(const vec2f a, const vec2f b, const vec2f c, const vec2f Q) {
  float abc_area = area(a, b, c);
  return uvw{area(Q, b, c) / abc_area, area(Q, c, a) / abc_area, area(Q, a, b) / abc_area};
}

inline vec2f closest_point_line(vec2f a, vec2f b, vec2f Q) {
  if (a == b) {
    return a;
  }
  vec2f n = (b - a);
  float l = length(n);
  n /= l;
  float v = (dot(Q - a, n)) / l;
  float u = (dot(b - Q, n)) / l;

  // uv uv = compute_uv_coords(A, B, Q);

  if (u <= 0) return b;
  if (v <= 0) return a;
  return a * u + b * v;
}

template<unsigned n>
static inline void bubble_sort_mapped(const float arr[], int u_2_s[]) {
  for (int i = 0; i < n - 1; i++) {
    for (int j = 0; j < n - i - 1; j++) {
      if (arr[u_2_s[j]] > arr[u_2_s[j + 1]]) {
        unsigned temp = u_2_s[j];
        u_2_s[j] = u_2_s[j + 1];
        u_2_s[j + 1] = temp;
      }
    }
  }
}

// returns the closest point in the direction d
inline vec2f poly_support_point(vec2f d, const vec2f *vertices, unsigned v_num) {
  assert(v_num > 0);
  unsigned max_index = 0;
  float max = -std::numeric_limits<float>::max();
  for (unsigned i = 0; i < v_num; i++) {
    float value = dot(vertices[i], d);
    if (value > max) {
      max = value;
      max_index = i;
    }
  }
  return vertices[max_index];
}

// returns the closest edges in the direction d
inline edge poly_support_edge(vec2f d, const vec2f *vertices, unsigned v_num) {
  unsigned first_max = 0;
  unsigned second_max = first_max;
  float max1 = -std::numeric_limits<float>::max();
  float max2 = -std::numeric_limits<float>::max();
  for (unsigned i = 0; i < v_num; i++) {
    float value = dot(vertices[i], d);
    if (value > max1) {
      max2 = max1;
      second_max = first_max;

      max1 = value;
      first_max = i;
    }
    if ((value < max1 && value > max2) || (value == max1 && i != first_max)) {
      max2 = value;
      second_max = i;
    }
  }
  return edge{vertices[first_max], vertices[second_max]};
}

// gjk
void pvp_np(
  const poly_cd &cd1,
  const poly_cd &cd2,
  unsigned c1,
  unsigned c2,
  contact &dst_contact1,
  contact &dst_contact2
) {
  simplex s;
  vec2f d = {0, 1};
  vec2f support_point;
  vec2f closest_point;
  vec2f last_closest_point = {0, 0};
  const vec2f O = {0, 0};

  const vec2f *a_data = cd1.get_vertices(c1);
  const vec2f *b_data = cd2.get_vertices(c2);
  unsigned a_v_num = cd1.vertex_num(c1);
  unsigned b_v_num = cd2.vertex_num(c2);

  support_point = poly_support_point(d, a_data, a_v_num) - poly_support_point(-d, b_data, b_v_num);
  s.add_vertex(support_point, d);
  while (true) {
    switch (s.count) {
    case (1): // simplex is a point
    {
      closest_point = s.v[0].p;
      d = O - s.v[0].p;
      break;
    }
    case (2): // simplex is a line
    {
      vec2f n = s.v[1].p - s.v[0].p;
      if (n == vec2f{0, 0}) {
        n = {1, 0};
      }
      float l = length(n);
      n /= l;
      float v = dot((O - s.v[0].p), n) / l;
      float u = dot((s.v[1].p - O), n) / l;

      if (u <= 0) {
        closest_point = s.v[1].p;
        d = O - closest_point;
        s.remove_vertex(0);
        break;
      }
      if (v <= 0) {
        closest_point = s.v[0].p;
        d = O - closest_point;
        s.remove_vertex(1);
        break;
      }

      closest_point = s.v[0].p * u + s.v[1].p * v;
      d = perp_l(s.v[1].p - s.v[0].p);
      break;
    }
    case (3): // simplex is a triangle
    {
      const vec2f a = s.v[0].p;
      const vec2f b = s.v[1].p;
      const vec2f c = s.v[2].p;

      const uv ab = compute_uv_coords(a, b, O);
      const uv bc = compute_uv_coords(b, c, O);
      const uv ca = compute_uv_coords(c, a, O);
      const uvw abc = compute_uvw_coords(a, b, c, O);

      if (abc.u > 0 && abc.v > 0 && abc.w > 0) {
        closest_point = O;
        break;
      }

      // vertex tests
      if (ab.v <= 0 && ca.u <= 0) {
        closest_point = a;
        d = O - closest_point;
        s.remove_vertex(1);
        s.remove_vertex(1);
        break;
      }
      if (bc.v <= 0 && ab.u <= 0) {
        closest_point = b;
        d = O - closest_point;
        s.remove_vertex(0);
        s.remove_vertex(1);
        break;
      }
      if (ca.v <= 0 && bc.u <= 0) {
        closest_point = c;
        d = O - closest_point;
        s.remove_vertex(0);
        s.remove_vertex(0);
        break;
      }
      // edge tests
      if (ab.u > 0 && ab.v > 0 && abc.w <= 0) {
        closest_point = a * ab.u + b * ab.v;
        d = perp_l(b - a);
        s.remove_vertex(2);
        break;
      }
      if (bc.u > 0 && bc.v > 0 && abc.u <= 0) {
        closest_point = b * bc.u + c * bc.v;
        d = perp_l(c - b);
        s.remove_vertex(0);
        break;
      }
      if (ca.u > 0 && ca.v > 0 && abc.v <= 0) {
        closest_point = c * ca.u + a * ca.v;
        d = perp_l(a - c);
        s.remove_vertex(1);
        break;
      }
      return;
    }
    }

    if (dot(d, (O - closest_point)) < 0) {
      d = -d;
    }
    if (closest_point == O) {
      break;
    }
    if (d == vec2f{0, 0}) {
      break;
    }
    if (length_squared(last_closest_point - closest_point) < 1e-5) {
      break;
    }

    last_closest_point = closest_point;
    support_point =
      poly_support_point(d, a_data, a_v_num) - poly_support_point(-d, b_data, b_v_num);
    s.add_vertex(support_point, d);
  }
  if (closest_point != O) {
    return;
  }
  /*
    EPA determines the normal, the penetration depth, and the vertices
    that make up the closest edge.
  */
  std::vector<simplex::simplex_vertex> polytope(3);
  float min_distance = std::numeric_limits<float>::max();
  vec2f min_normal = {};
  unsigned polytope_edge_index1 = 0;
  unsigned polytope_edge_index2 = 0;

  polytope[0] = s.v[0];
  polytope[1] = s.v[1];
  polytope[2] = s.v[2];

  // If gjk just returns a line create new point that makes the simplex that contains the origin.
  if (polytope[2].p == vec2f{0, 0}) {
    vec2f new_d = perp_r(polytope[1].p - polytope[0].p);
    vec2f new_support =
      poly_support_point(new_d, a_data, a_v_num) - poly_support_point(-new_d, b_data, b_v_num);

    polytope[2].d = new_d;
    polytope[2].p = new_support;
  }

  while (min_distance == std::numeric_limits<float>::max()) {
    unsigned min_index1 = 0;
    unsigned min_index2 = 0;
    for (unsigned i = 0; i < polytope.size(); i++) {
      unsigned j = (i + 1) % polytope.size();

      vec2f normal = perp_l(polytope[j].p - polytope[i].p);
      float normal_length = length(normal);
      if (normal_length == 0) {
        normal = {1, 0};
      } else {
        normal /= normal_length;
      }
      float distance = dot(normal, polytope[i].p);

      if (distance < 0) {
        distance *= -1;
        normal *= -1.f;
      }

      if (distance < min_distance) {
        min_distance = distance;
        min_normal = normal;
        min_index1 = i;
        min_index2 = j;
      }
    }

    vec2f support = poly_support_point(min_normal, a_data, a_v_num) -
                    poly_support_point(-min_normal, b_data, b_v_num);

    float sdistance = dot(min_normal, support);

    if (sdistance - min_distance > 0.001) {
      min_distance = std::numeric_limits<float>::max();
      polytope.insert(polytope.begin() + min_index2, simplex::simplex_vertex{support, min_normal});
    } else {
      polytope_edge_index1 = min_index1;
      polytope_edge_index2 = min_index2;
      break;
    }

    if (polytope.size() > 20) {
      break;
    }
  }
  // Figure out if there is an edge completely crossing another edge
  edge a_e = poly_support_edge(min_normal, a_data, a_v_num);
  edge b_e = poly_support_edge(-min_normal, b_data, b_v_num);

  // normal's basis
  const mat2f nb = {-min_normal.y, min_normal.x, min_normal.x, min_normal.y};

  // converting the points to the normals basis
  const vec2f nb_aa = nb * a_e.a;
  const vec2f nb_ab = nb * a_e.b;
  const vec2f nb_ba = nb * b_e.a;
  const vec2f nb_bb = nb * b_e.b;

  vec2f e_points[4] = {a_e.a, a_e.b, b_e.a, b_e.b};
  float n_points_x[4] = {nb_aa.x, nb_ab.x, nb_ba.x, nb_bb.x};
  float n_points_y[4] = {nb_aa.y, nb_ab.y, nb_ba.y, nb_bb.y};

  // unsorted to sorted
  int u_t_sx[4] = {0, 1, 2, 3};
  int u_t_sy[4] = {0, 1, 2, 3};
  bubble_sort_mapped<4>(n_points_x, u_t_sx);
  bubble_sort_mapped<4>(n_points_y, u_t_sy);

  bool edge_is_crossing = (u_t_sy[2] == 0 || u_t_sy[2] == 1) && (u_t_sy[3] == 0 || u_t_sy[3] == 1);

  if ((!edge_is_crossing || min_distance == 0) && min_distance < 100) {
    // single contact point
    vec2f a_1 = poly_support_point(polytope[polytope_edge_index1].d, a_data, a_v_num);
    vec2f b_1 = poly_support_point(-polytope[polytope_edge_index1].d, b_data, b_v_num);
    vec2f a_2 = poly_support_point(polytope[polytope_edge_index2].d, a_data, a_v_num);
    vec2f b_2 = poly_support_point(-polytope[polytope_edge_index2].d, b_data, b_v_num);
    // Reconstruct the Minkowski difference vertices (should closely match polytope vertices)
    vec2f v0 = a_1 - b_1;
    vec2f v1 = a_2 - b_2;

    // Compute the edge vector of the closest edge
    vec2f edge = v1 - v0;
    float edge_length_sq = length_squared(edge);

    // Compute barycentric coordinate 't' for the projection of the origin onto the edge
    // This calculates how far along the edge the projection lies (clamped to [0,1])
    float t = (edge_length_sq != 0) ? -dot(v0, edge) / edge_length_sq : 0.0f;
    t = std::max(0.0f, std::min(1.0f, t));

    // Interpolate the witness points from each shape using the barycentrics
    vec2f contact_a = a_1 * (1 - t) + a_2 * t;
    vec2f contact_b = b_1 * (1 - t) + b_2 * t;

    dst_contact1 = contact{contact_a, contact_b, min_normal};
  } else {

    /*
      1. project points onto a's edge.
      2. calculate closest distance from these new points to b's edge to get the depth
      3. set the values
      (the following isn't optimal)
    */

    vec2f a_edge = (e_points[1] - e_points[0]);
    a_edge /= length(a_edge);
    vec2f poc_a = e_points[0] + a_edge * dot((e_points[u_t_sx[1]] - e_points[0]), a_edge);
    vec2f poc_b = e_points[0] + a_edge * dot((e_points[u_t_sx[2]] - e_points[0]), a_edge);

    float depth_a = length(closest_point_line(e_points[3], e_points[2], poc_a) - poc_a);
    float depth_b = length(closest_point_line(e_points[3], e_points[2], poc_b) - poc_b);

    dst_contact1 = contact{poc_a, poc_a - min_normal * depth_a, min_normal};
    dst_contact2 = contact{poc_b, poc_b - min_normal * depth_b, min_normal};
  }
};
} // namespace cp6
