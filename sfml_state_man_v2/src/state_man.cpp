#include "state_man.h"
#include "state.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

key *state_man::get_key(std::string key_name) {
  key_map.lock();
  key *key = m_supported_keys[key_name];
  key_map.unlock();
  return key;
}

state_man::state_man(std::string dir) : dir(dir) {
  create_default_config(dir);
  init_keys();
  init_window();
}

state_man::~state_man() {
  for (size_t i = 0; i < m_keys.size(); i++) {
    delete m_keys[i];
  }
}

namespace fs = std::filesystem;

void state_man::create_default_config(std::string config_dir) {
  config_dir.append("/config");
  // Create the config directory if it doesn't exist
  if (!fs::exists(config_dir)) {
    if (!fs::create_directory(config_dir)) {
      std::cerr << "Failed to create config directory: " << config_dir << std::endl;
      return;
    }
  }

  // Define the path to the window.ini file
  std::string config_file_path = config_dir + "/window.ini";

  // Write default settings to the file only if it doesn't exist
  if (!fs::exists(config_file_path)) {
    std::ofstream ofs(config_file_path);
    if (!ofs.is_open()) {
      std::cerr << "Failed to create or open config file: " << config_file_path << std::endl;
      return;
    }
    // Write default settings to the file
    ofs << "My Game Title\n"; // Default window title
    ofs << "800 500\n";       // Default window size (width height)
    ofs << "165\n";           // Default framerate limit
    ofs << "0\n";             // Default vertical sync setting (0 for disabled, 1 for enabled)
    ofs.close();
    std::cout << "Created default config file: " << config_file_path << std::endl;
  }

  // Define the path to the supported_keys.ini file
  config_file_path = config_dir + "/supported_keys.ini";

  // Write default key bindings to the file only if it doesn't exist
  if (!fs::exists(config_file_path)) {
    std::ofstream ofs(config_file_path);
    if (!ofs.is_open()) {
      std::cerr << "Failed to create or open config file: " << config_file_path << std::endl;
      return;
    }
    // Write default key bindings to the file
    ofs << "A 0\n";
    ofs << "B 1\n";
    ofs << "C 2\n";
    ofs << "D 3\n";
    ofs << "E 4\n";
    ofs << "F 5\n";
    ofs << "G 6\n";
    ofs << "H 7\n";
    ofs << "I 8\n";
    ofs << "J 9\n";
    ofs << "K 10\n";
    ofs << "L 11\n";
    ofs << "M 12\n";
    ofs << "N 13\n";
    ofs << "O 14\n";
    ofs << "P 15\n";
    ofs << "Q 16\n";
    ofs << "R 17\n";
    ofs << "S 18\n";
    ofs << "T 19\n";
    ofs << "U 20\n";
    ofs << "V 21\n";
    ofs << "W 22\n";
    ofs << "X 23\n";
    ofs << "Y 24\n";
    ofs << "Z 25\n";
    ofs << "Num0 26\n";
    ofs << "Num1 27\n";
    ofs << "Num2 28\n";
    ofs << "Num3 29\n";
    ofs << "Num4 30\n";
    ofs << "Num5 31\n";
    ofs << "Num6 32\n";
    ofs << "Num7 33\n";
    ofs << "Num8 34\n";
    ofs << "Num9 35\n";
    ofs << "Escape 36\n";
    ofs << "LControl 37\n";
    ofs << "LShift 38\n";
    ofs << "LAlt 39\n";
    ofs << "LSystem 40\n";
    ofs << "RControl 41\n";
    ofs << "RShift 42\n";
    ofs << "RAlt 43\n";
    ofs << "RSystem 44\n";
    ofs << "Menu 45\n";
    ofs << "LBracket 46\n";
    ofs << "RBracket 47\n";
    ofs << "Semicolon 48\n";
    ofs << "Comma 49\n";
    ofs << "Period 50\n";
    ofs << "Quote 51\n";
    ofs << "Slash 52\n";
    ofs << "Backslash 53\n";
    ofs << "Tilde 54\n";
    ofs << "Equal 55\n";
    ofs << "Hyphen 56\n";
    ofs << "Space 57\n";
    ofs << "Enter 58\n";
    ofs << "Backspace 59\n";
    ofs << "Tab 60\n";
    ofs << "PageUp 61\n";
    ofs << "PageDown 62\n";
    ofs << "End 63\n";
    ofs << "Home 64\n";
    ofs << "Insert 65\n";
    ofs << "Delete 66\n";
    ofs << "Add 67\n";
    ofs << "Subtract 68\n";
    ofs << "Multiply 69\n";
    ofs << "Divide 70\n";
    ofs << "Left 71\n";
    ofs << "Right 72\n";
    ofs << "Up 73\n";
    ofs << "Down 74\n";
    ofs << "Numpad0 75\n";
    ofs << "Numpad1 76\n";
    ofs << "Numpad2 77\n";
    ofs << "Numpad3 78\n";
    ofs << "Numpad4 79\n";
    ofs << "Numpad5 80\n";
    ofs << "Numpad6 81\n";
    ofs << "Numpad7 82\n";
    ofs << "Numpad8 83\n";
    ofs << "Numpad9 84\n";
    ofs << "F1 85\n";
    ofs << "F2 86\n";
    ofs << "F3 87\n";
    ofs << "F4 88\n";
    ofs << "F5 89\n";
    ofs << "F6 90\n";
    ofs << "F7 91\n";
    ofs << "F8 92\n";
    ofs << "F9 93\n";
    ofs << "F10 94\n";
    ofs << "F11 95\n";
    ofs << "F12 96\n";
    ofs << "F13 97\n";
    ofs << "F14 98\n";
    ofs << "F15 99\n";
    ofs << "Pause 100\n";
    ofs.close();
    std::cout << "Created default key bindings file: " << config_file_path << std::endl;
  }

  std::cout << "Config directory is ready: " << config_dir << std::endl;
}

void state_man::init_keys() {
  std::ifstream ifs(dir + "/config/supported_keys.ini");

  if (ifs.is_open()) {
    std::string key_name = "";
    int key_value = 0;

    while (ifs >> key_name >> key_value) {
      key *nkey = new key(key_value);
      this->m_supported_keys[key_name] = nkey;
      m_keys.push_back(nkey);
    }
  }else{
    std::cerr << "no supported_keys.ini files at: " << dir + "/config" << std::endl;
    assert(false);
  }
  ifs.close();
}

void state_man::init_window() {
  std::ifstream ifs(dir + "/config/window.ini");

  std::string title = "None";

  sf::VideoMode window_bounds({800, 500});
  unsigned int framerate_limit = 165;
  bool vertical_sync_enabled = false;
  assert(ifs.is_open());
  if (ifs.is_open()) {
    std::getline(ifs, title);
    ifs >> window_bounds.size.x >> window_bounds.size.y;
    ifs >> framerate_limit;
    ifs >> vertical_sync_enabled;
  }
  ifs.close();
  m_window = sf::RenderWindow(
    window_bounds,
    title
  ); // idk what "this" means it is just creating a heap variable from the pointer "window". 4/10
  if (framerate_limit != 0) {
    m_window.setFramerateLimit(framerate_limit);
  }

  m_window.setVerticalSyncEnabled(vertical_sync_enabled);

  this->m_default_view = m_window.getDefaultView();
}

void state_man::end_app(std::string reason) {
  this->end_reason = reason;

  quit = true;
}

void state_man::check_end() {
  if (this->quit == true) {
    std::cout << "Ending Application" << "\n";
    std::cout << "Reason: " << end_reason << "\n";

    for (size_t i = 0; i < m_states.size(); i++) {
      delete m_states[i]; // PS if you get an error here you messed up the memory in the state.
    }
    this->m_window.close();
  }
}
void state_man::update_mouse() {
  m_mouse_poss_screen = sf::Mouse::getPosition();
  m_mouse_pos_window = sf::Mouse::getPosition(m_window);
}

void state_man::update_keys() {
  for (unsigned int i = 0; i < m_keys.size(); i++) {
    key *ckey = this->m_keys[i];
    ckey->isreleased = 0;

    if (sf::Keyboard::isKeyPressed(ckey->boundkey)) {
      ckey->ispressed = 1;
      if (ckey->pressedlast == 1) {
        ckey->ispressed = 0;
        ckey->isheld = 1;
        ckey->timeheld = ckey->time.getElapsedTime().asSeconds();
      }
      ckey->pressedlast = 1;
    } else {
      if (ckey->isheld) {
        ckey->isreleased = 1;
      }
      ckey->ispressed = 0;
      ckey->pressedlast = 0;
      ckey->isheld = 0;
      ckey->timeheld = 0;
      ckey->time.restart();
    }
  }
}

void state_man::update_window() {
  while (const std::optional event = m_window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
      end_app("User Closed Window");
      return;
    }
    if (event->is<sf::Event::Resized>()) {
      std::cout << "Window Resized" << "\n";
      // this->m_window.setView(m_window.getDefaultView());
      for (size_t i = 0; i < m_states.size(); i++) {
        sf::Vector2f scale_factor;
        scale_factor.x = m_window.getSize().x / m_states[i]->m_view.getSize().x;
        scale_factor.y = m_window.getSize().y / m_states[i]->m_view.getSize().y;

        sf::Vector2f new_size(
          scale_factor.x * m_states[i]->m_view.getSize().x,
          scale_factor.y * m_states[i]->m_view.getSize().y
        );

        m_states[i]->m_view.setSize(new_size);
      }
      m_default_view.setSize(sf::Vector2f(m_window.getSize()));
    }
  }
  m_window.clear();
  sf::RectangleShape layer_fog;

  layer_fog.setFillColor(sf::Color(50, 50, 50, 50));
  layer_fog.setSize(sf::Vector2f(get_window_size()));
  layer_fog.setPosition(
    m_default_view.getCenter() - sf::Vector2f(get_window_size().x / 2.f, get_window_size().y / 2.f)
  );

  std::sort(m_draw_stack.begin(), m_draw_stack.end(), [&](const unsigned a, const unsigned b) {
    return m_states[a]->draw_priority < m_states[b]->draw_priority;
  });

  for (size_t i = 0; i < m_draw_stack.size(); i++) {
    if (i > 0) {
      m_window.draw(layer_fog);
    }

    m_window.setView(m_states[m_draw_stack[i]]->m_view);
    m_states[m_draw_stack[i]]->draw(m_window);
    m_window.setView(this->get_default_view());
  }
  m_draw_stack.clear();
  m_window.display();
}

void state_man::update_dt() {
  this->dt = this->dtClock.getElapsedTime().asSeconds();
  this->dtClock.restart();
}

void state_man::update_state(state *state) {
  state->update();
}

void state_man::remove_state(unsigned short int index) {
  delete m_states[index];

  for (size_t i = index; i < m_states.size() - 1; i++) {
    m_states[i] = m_states[i + 1];
  }
  m_states.pop_back();
}

bool state_man::is_quit() { return quit; }

void state_man::update() {
  update_dt();
  update_mouse();
  update_keys();
  update_states();
  update_window();
  check_end();
}

void state_man::update_states() {
  /*
  update, draw, update and draw, neither
  */
  // update back

  if (m_states.size() < 1) {
    end_app("No More States");
    return;
  }
  // update background states
  state_vector.lock();
  for (unsigned int i = 0; i < m_states.size() - 1; i++) {
    switch (m_states[i]->background_state) {
    case (Update): {
      m_states[i]->dt = this->dt;
      m_update_futures.push_back(
        std::async(std::launch::async, update_state, m_states[i])
      );
      break;
    }
    case (Update_Draw): {
      m_states[i]->dt = this->dt;
      m_update_futures.push_back(
        std::async(std::launch::async, update_state, m_states[i])
      );
      break;
    }
    case (Wait):
      break;
    case (Remove):
      break;
    case (Draw):
      break;
    }
  }
  state_vector.unlock();
  // update main state
  update_state(m_states.back());

  for (size_t i = 0; i < m_update_futures.size(); i++) {
    m_update_futures[i].wait();
  }

  m_update_futures.clear();

  for (unsigned i = 0; i < m_states.size(); i++) {
    m_states[i]->dt = this->dt;
    switch (m_states[i]->background_state) {
    case (Remove): {
      remove_state(i);
      i--;
      break;
    }
    case (Draw): {
      m_draw_stack.push_back(i);
      break;
    }
    case (Update_Draw): {
      m_draw_stack.push_back(i);
      break;
    }
    case (Update): {
      break;
    }
    case (Wait): {
      break;
    }
    default: {
      remove_state(i);
      i--;
      std::cout << "unrecognizable background state" << "\n";
    }
    }
  }
}
