#pragma once
#include "state.h"
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <state_man.h>

class flag_state: public state {
public:
  flag_state(state_man &man);
  void update() override;
  void draw(sf::RenderWindow &window) override;

private:
  void load_font();
  void update_text();

  sf::Font m_font;
  sf::Vector2f m_text_pos;
  std::vector<sf::Text> m_texts;
  unsigned cursor = 0;
};
