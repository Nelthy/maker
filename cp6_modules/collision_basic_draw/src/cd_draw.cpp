#include "collision_basic_draw/draw_cd.h"
#include "collision_basic_draw/to_sf.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>

void draw_cd(sf::RenderWindow &window, const cp6::circle_cd &cd, sf::Color color) {
  sf::CircleShape circle;
  circle.setFillColor(color);
  circle.setOutlineThickness(-1);

  for (unsigned q = 0; q < cd.size(); q++) {
    circle.setRadius(cd[q].radius);
    circle.setOrigin({circle.getRadius(), circle.getRadius()});
    circle.setPosition(to_sf(cd[q].pos));
    window.draw(circle);
  }
}

void draw_cd(sf::RenderWindow &window, const cp6::poly_cd &cd, sf::Color color) {
  std::vector<sf::Vertex> vertices;
  for (unsigned q = 0; q < cd.size(); q++) {
    unsigned q_v_size = cd.vertex_num(q);
    if (q_v_size > 0) {
      const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
      vertices.resize(q_v_size);
      for (unsigned i = 0; i < vertices.size(); i++) {
        vertices[i].position = to_sf(q_tcd[i]);
        vertices[i].color = color;
      }
      window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::TriangleFan);
      vertices.push_back(vertices[0]);
      for (unsigned i = 0; i < vertices.size(); i++) {
        vertices[i].color = sf::Color::White;
      }
      window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
    }
  }
}

void draw_cd(
  sf::RenderWindow &window,
  const cp6::circle_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  sf::CircleShape circle;
  circle.setFillColor(color);
  circle.setOutlineThickness(-1);

  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    unsigned q = ind[ind_q];
    circle.setRadius(cd[q].radius);
    circle.setOrigin({circle.getRadius(), circle.getRadius()});
    circle.setPosition(to_sf(cd[q].pos));
    window.draw(circle);
  }
}

void draw_cd(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  std::vector<sf::Vertex> vertices;
  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    unsigned q = ind[ind_q];
    unsigned q_v_size = cd.vertex_num(q);
    if (q_v_size > 0) {
      const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
      vertices.resize(q_v_size);
      for (unsigned i = 0; i < vertices.size(); i++) {
        vertices[i].position = to_sf(q_tcd[i]);
        vertices[i].color = color;
      }
      window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::TriangleFan);
      vertices.push_back(vertices[0]);
      for (unsigned i = 0; i < vertices.size(); i++) {
        vertices[i].color = sf::Color::White;
      }
      window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
    }
  }
}

void draw_cd_outline(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color
) {
  std::vector<sf::Vertex> vertices;
  for (unsigned ind_q = 0; ind_q < ind_size; ind_q++) {
    unsigned q = ind[ind_q];
    unsigned q_v_size = cd.vertex_num(q);
    if (q_v_size > 0) {
      const vec2f *q_tcd = &cd.m_transformed_cd[cd.to_transformed_cd()[q]];
      vertices.resize(q_v_size);
      for (unsigned i = 0; i < vertices.size(); i++) {
        vertices[i].position = to_sf(q_tcd[i]);
        vertices[i].color = color;
      }
      vertices.push_back(vertices[0]);
      window.draw(vertices.data(), vertices.size(), sf::PrimitiveType::LineStrip);
    }
  }
}
