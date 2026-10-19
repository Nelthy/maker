#pragma once
#include "collision_basic/contact_set.h"
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

void draw_col_state(
  sf::RenderWindow &window,
  const cp6::contact_1p_set &state,
  vec2f global_offset,
  sf::Color a_color = {255, 0, 0, 200},
  sf::Color b_color = {0, 0, 255, 200}

);
void draw_col_state(
  sf::RenderWindow &window,
  const cp6::contact_2p_set &state,
  vec2f global_offset,
  sf::Color a_color = {255, 0, 0, 200},
  sf::Color b_color = {0, 0, 255, 200}
);
