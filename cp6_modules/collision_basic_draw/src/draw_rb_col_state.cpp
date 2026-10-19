#include "collision_basic_draw/draw_rb_col_state.h"
#include "collision_basic_draw/to_sf.h"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Vector2.hpp>

void draw_col_state(
  sf::RenderWindow &window,
  const cp6::contact_1p_set&col_state,
  vec2f global_offset,
  sf::Color a_color,
  sf::Color b_color
) {
  sf::CircleShape circle;
  circle.setOutlineThickness(-1);
  circle.setFillColor(sf::Color::Red);
  circle.setRadius(5);
  circle.setOrigin({circle.getRadius(), circle.getRadius()});

  std::array<sf::Vertex, 2> arrow;
  arrow[0].color = a_color;
  arrow[1].color = b_color;

  for (unsigned i = 0; i < col_state.size(); i++) {
    sf::Vector2f x1 = to_sf(col_state[i].x1 + global_offset);
    sf::Vector2f x2 = to_sf(col_state[i].x2 + global_offset);
    sf::Vector2f n = to_sf(col_state[i].n);

    circle.setFillColor(a_color);
    circle.setPosition(x1);
    window.draw(circle);

    circle.setFillColor(b_color);
    circle.setPosition(x2);
    window.draw(circle);

    arrow[0].position = x1;
    arrow[1].position = x1 + n * 25.f;
    window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
  }
};

void draw_col_state(
  sf::RenderWindow &window,
  const cp6::contact_2p_set &col_state,
  vec2f global_offset,
  sf::Color a_color,
  sf::Color b_color
) {
  sf::CircleShape circle;
  circle.setOutlineThickness(-1);
  circle.setFillColor(sf::Color::Red);
  circle.setRadius(5);
  circle.setOrigin({circle.getRadius(), circle.getRadius()});

  std::array<sf::Vertex, 2> arrow;
  arrow[0].color = a_color;
  arrow[1].color = b_color;

  for (unsigned i = 0; i < col_state.size(); i++) {
    sf::Vector2f x1 = to_sf(col_state[i].cnt1.x1 + global_offset);
    sf::Vector2f x2 = to_sf(col_state[i].cnt1.x2 + global_offset);
    sf::Vector2f n = to_sf(col_state[i].cnt1.n);

    circle.setFillColor(a_color);
    circle.setPosition(x1);
    window.draw(circle);

    circle.setFillColor(b_color);
    circle.setPosition(x2);
    window.draw(circle);

    arrow[0].position = x1;
    arrow[1].position = x1 + n * 25.f;
    window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
  }

  for (unsigned i = 0; i < col_state.size(); i++) {
    if (!col_state[i].cnt2.is_valid()) continue;

    sf::Vector2f x1 = to_sf(col_state[i].cnt2.x1 + global_offset);
    sf::Vector2f x2 = to_sf(col_state[i].cnt2.x2 + global_offset);
    sf::Vector2f n = to_sf(col_state[i].cnt2.n);

    circle.setFillColor(a_color);
    circle.setPosition(x1);
    window.draw(circle);

    circle.setFillColor(b_color);
    circle.setPosition(x2);
    window.draw(circle);

    arrow[0].position = x1;
    arrow[1].position = x1 + n * 25.f;
    window.draw(arrow.data(), arrow.size(), sf::PrimitiveType::Lines);
  }
};
