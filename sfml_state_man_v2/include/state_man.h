#pragma once
#include "state.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <future>
#include <map>
#include <string>
#include <vector>

class state_man {
public:
  template<typename T, typename... TArgs>
    requires std::is_base_of_v<state, T>
  void push_new_state(TArgs &&...m_Args) {
    T *state = new T(*this, std::forward<TArgs>(m_Args)...);
    state_vector.lock();
    m_states.push_back(state);
    state_vector.unlock();
  }

  key *get_key(std::string key_name);

  state_man(std::string dir);
  ~state_man();

  void end_app(std::string reason);

  const sf::RenderWindow &get_window() { return m_window; };

  sf::Vector2u get_window_size() { return m_window.getSize(); };

  bool is_quit();

  inline sf::Vector2i mouse_pos_screen() const { return this->m_mouse_poss_screen; }

  inline sf::Vector2i mouse_pos_window() const { return this->m_mouse_pos_window; }

  inline const sf::View get_default_view() const { return m_default_view; }

  const float *get_dt() { return &this->dt; }

  std::string get_dir() { return this->dir; }

  unsigned state_num() { return m_states.size(); }

private:
  friend class app;

  void create_default_config(std::string config_dir);

  void update();

  void update_states();

  void init_keys();

  void init_window();

  void update_mouse();

  void update_keys();

  void update_window();

  void update_dt();

  static void update_state(state *state);

  void remove_state(unsigned short int index);

  void check_end();
  std::string end_reason;
  std::string dir;
  sf::View m_default_view{};
  sf::Vector2i m_mouse_poss_screen{};
  sf::Vector2i m_mouse_pos_window{};

  std::mutex state_vector;
  std::mutex key_map;

  std::vector<state *> m_states;

  std::vector<unsigned int> m_draw_stack;

  std::map<std::string, key *> m_supported_keys;
  std::vector<key *> m_keys;

  sf::RenderWindow m_window;
  sf::Image m_icon;

  std::vector<std::future<void>> m_update_futures;

  sf::Clock dtClock;
  bool quit = 0;
  float dt = 0;
};
