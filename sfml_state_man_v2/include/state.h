#pragma once
#include "SFML/Graphics.hpp"
#include "key.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <map>

enum activeType {
  Wait,
  Remove,
  Update,
  Draw,
  Update_Draw,
};

class state_man;

class state {
public:
  sf::View m_view;
  float draw_priority = 0;
  sf::Vector2f mouse_pos_view();
  float dt = 0;

  activeType background_state = Update_Draw;

  state_man &m_man;

  std::map<std::string, key *> keybinds;

  bool load_keys_from_config(std::string file);

  virtual void update() = 0;
  virtual void draw(sf::RenderWindow &window) = 0;

  state(state_man &app);
  virtual ~state();
};
#include "state_man.h"
