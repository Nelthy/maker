#include "collision_basic_draw/draw_cd.h"
#include "collision_basic_draw/to_sf.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <numbers>

// I had a clanker remake these to not be so dumb.
namespace {
constexpr unsigned circle_point_count = 24;
const std::array<sf::Vector2f, circle_point_count + 1> &unit_circle_points() {
  static const auto points = [] {
    std::array<sf::Vector2f, circle_point_count + 1> result{};
    constexpr float two_pi = 2.0f * std::numbers::pi_v<float>;
    for (unsigned i = 0; i <= circle_point_count; i++) {
      const float angle = two_pi * static_cast<float>(i) / circle_point_count;
      result[i] = {std::cos(angle), std::sin(angle)};
    }
    return result;
  }();
  return points;
}

void append_circle_fill(
  std::vector<sf::Vertex> &out,
  sf::Vector2f center,
  float radius,
  sf::Color color
) {
  const auto &unit = unit_circle_points();
  for (unsigned i = 0; i < circle_point_count; i++) {
    out.push_back({center, color});
    out.push_back({center + unit[i] * radius, color});
    out.push_back({center + unit[i + 1] * radius, color});
  }
}

// convex polygon, fan -> triangle list
void append_poly_fill(
  std::vector<sf::Vertex> &out,
  const vec2f *tcd,
  unsigned n,
  sf::Color color
) {
  const sf::Vector2f first = to_sf(tcd[0]);
  for (unsigned i = 1; i + 1 < n; i++) {
    out.push_back({first, color});
    out.push_back({to_sf(tcd[i]), color});
    out.push_back({to_sf(tcd[i + 1]), color});
  }
}

// closed line strip -> line list, one segment per edge
void append_poly_outline(
  std::vector<sf::Vertex> &out,
  const vec2f *tcd,
  unsigned n,
  sf::Color color
) {
  if (n < 2) { return; }
  for (unsigned i = 0; i < n; i++) {
    const unsigned next = (i + 1 == n) ? 0 : i + 1;
    out.push_back({to_sf(tcd[i]), color});
    out.push_back({to_sf(tcd[next]), color});
  }
}

void draw(
  sf::RenderWindow &window,
  const std::vector<sf::Vertex> &vertices,
  sf::PrimitiveType type
) {
  if (!vertices.empty()) {
    window.draw(vertices.data(), vertices.size(), type);
  }
}
} // namespace

void draw_cd(
  sf::RenderWindow &window,
  const cp6::circle_cd &cd,
  sf::Color color
) {
  static std::vector<sf::Vertex> vertices;
  vertices.clear();

  for (unsigned q = 0; q < cd.size(); q++) {
    append_circle_fill(vertices, to_sf(cd[q].pos), cd[q].radius, color);
  }
  draw(window, vertices, sf::PrimitiveType::Triangles);
}

void draw_cd(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  sf::Color color
) {
  static std::vector<sf::Vertex> fill_vertices;
  static std::vector<sf::Vertex> line_vertices;
  fill_vertices.clear();
  line_vertices.clear();

  for (unsigned q = 0; q < cd.size(); q++) {
    const unsigned n = cd.vertex_num(q);
    if (n == 0) { continue; }
    const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
    append_poly_fill(fill_vertices, q_tcd, n, color);
    append_poly_outline(line_vertices, q_tcd, n, sf::Color::White);
  }
  draw(window, fill_vertices, sf::PrimitiveType::Triangles);
  draw(window, line_vertices, sf::PrimitiveType::Lines);
}

void draw_cd(
  sf::RenderWindow &window,
  const cp6::circle_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  static std::vector<sf::Vertex> vertices;
  vertices.clear();

  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    const unsigned q = ind[ind_q];
    append_circle_fill(vertices, to_sf(cd[q].pos), cd[q].radius, color);
  }
  draw(window, vertices, sf::PrimitiveType::Triangles);
}

void draw_cd(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  static std::vector<sf::Vertex> fill_vertices;
  static std::vector<sf::Vertex> line_vertices;
  fill_vertices.clear();
  line_vertices.clear();

  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    const unsigned q = ind[ind_q];
    const unsigned n = cd.vertex_num(q);
    if (n == 0) { continue; }
    const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
    append_poly_fill(fill_vertices, q_tcd, n, color);
    append_poly_outline(line_vertices, q_tcd, n, sf::Color::White);
  }
  draw(window, fill_vertices, sf::PrimitiveType::Triangles);
  draw(window, line_vertices, sf::PrimitiveType::Lines);
}

void draw_cd_outline(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  static std::vector<sf::Vertex> line_vertices;
  line_vertices.clear();

  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    const unsigned q = ind[ind_q];
    const unsigned n = cd.vertex_num(q);
    if (n == 0) { continue; }
    const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
    append_poly_outline(line_vertices, q_tcd, n, color);
  }
  draw(window, line_vertices, sf::PrimitiveType::Lines);
}
