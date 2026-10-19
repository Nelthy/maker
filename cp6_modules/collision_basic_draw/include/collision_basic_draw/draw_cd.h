#include "collision_basic/circle_cd.h"
#include "collision_basic/poly_cd.h"
#include <SFML/Graphics/RenderWindow.hpp>

void draw_cd(
  sf::RenderWindow &window,
  const cp6::circle_cd &cd,
  sf::Color color = sf::Color::Magenta
);

void draw_cd(sf::RenderWindow &window, const cp6::poly_cd &cd, sf::Color color = sf::Color::Yellow);

void draw_cd_outline(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  sf::Color color = sf::Color::White
);

void draw_cd(
  sf::RenderWindow &window,
  const cp6::circle_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color = sf::Color::Magenta
);

void draw_cd(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color = sf::Color::Yellow
);

void draw_cd_outline(
  sf::RenderWindow &window,
  const cp6::poly_cd &cd,
  const unsigned *ind,
  unsigned ind_size,
  sf::Color color = sf::Color::White
);
