#include "state.h"
#include <SFML/System/Vector2.hpp>
#include <fstream>
#include <iostream>

sf::Vector2f state::mouse_pos_view() {
  return sf::Vector2f(this->m_man.mouse_pos_window()) -
         (sf::Vector2f(m_man.get_window().getSize()) * 0.5f +
          m_view.getCenter());
}

state::state(state_man &manager)
    : m_view(manager.get_default_view()), m_man(manager) {}

state::~state() {}

bool state::load_keys_from_config(std::string file) {
  std::string config_path = m_man.get_dir().append("/config/");
  file = config_path.append(file);
  std::cout << "loading keys from file: " << config_path << "\n";
  std::ifstream ifs(file);
  if (ifs.is_open()) {
    std::string key = "";
    std::string key2 = "";

    while (ifs >> key >> key2) {
      this->keybinds[key] = m_man.get_key(key2);
    }
    return true;
  } else {
    return false;
  }
}
