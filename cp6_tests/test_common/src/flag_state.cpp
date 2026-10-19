#include "flag_state.h"
#include "flags.h"
#include "scalers.h"
#include "state_man.h"
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
#include <iostream>

flag_state::flag_state(state_man &man) : state(man) {
  // load your flag keys, same as before
  if (!load_keys_from_config("flag_keys.ini")) {
    std::cerr << "Failed to load flag_keys.ini\n";
    assert(false);
  }

  // you need a .ttf in your resources folder
  if (!m_font.openFromFile(m_man.get_dir().append("/assets/ProggyClean.ttf"))) {
    std::cerr << "Failed to load font\n";
    assert(false);
  }
  update_text();
  draw_priority = 100;
}

void flag_state::update() {
  static float last_flag_num = 0;
  static float last_scaler_num = 0;

  if (last_flag_num != flags::num() || last_scaler_num != scalers::num()) {
    cursor = 0;
  }
  last_flag_num = flags::num();
  last_scaler_num = scalers::num();

  update_text();
  if (m_man.state_num() == 1) {
    background_state = Remove;
  }
  static float elapsed = 0;
  elapsed += dt;
  if (elapsed > 0.03) {
    elapsed = 0;
  }
  if (keybinds["down"]->ispressed || (keybinds["down"]->timeheld > 0.2 && elapsed == 0)) {
    if (cursor + 1 < flags::num() + scalers::num()) ++cursor;
    update_text();
  }

  if (keybinds["up"]->ispressed || (keybinds["up"]->timeheld > 0.2 && elapsed == 0)) {
    if (cursor > 0) --cursor;
    update_text();
  }

  if ((keybinds["toggle"]->ispressed || (keybinds["toggle"]->timeheld > 0.2 && elapsed == 0)) &&
      !keybinds["modify"]->isheld) {
    if (cursor < flags::num() + scalers::num()) {
      if (cursor < flags::num()) {
        flags::toggle(cursor);
      } else {
        scalers::increment(cursor - flags::num());
      }
    }
    update_text();
  }
  if ((keybinds["toggle"]->ispressed || (keybinds["toggle"]->timeheld > 0.2) && elapsed == 0) &&
      keybinds["modify"]->isheld) {
    if (cursor < flags::num() + scalers::num()) {
      if (cursor < flags::num()) {
        flags::toggle(cursor);
      } else {
        scalers::decrement(cursor - flags::num());
      }
    }
    update_text();
  }
}

void flag_state::update_text() {
  m_texts.clear();
  m_texts.reserve(flags::num());

  const float line_height =
    m_font.getLineSpacing(16); // ~font size + padding :contentReference[oaicite:0]{index=0}
  const unsigned font_size = 16;

  for (unsigned i = 0; i < flags::num(); ++i) {
    sf::Text txt(m_font);

    txt.setCharacterSize(font_size);
    txt.setString(flags::get_name(i) + ": " + (flags::get(i) ? "true" : "false"));
    // position so that line 0 is at top, line N below
    txt.setPosition({10.f, 10.f + i * line_height});
    txt.setPosition(txt.getPosition() + m_text_pos);
    // selected line in bold or different color
    if (i == cursor) {
      txt.setStyle(sf::Text::Bold);
      txt.setFillColor(sf::Color::Yellow);
    }
    m_texts.push_back(std::move(txt));
  }
  for (unsigned i = 0; i < scalers::num(); ++i) {
    sf::Text txt(m_font);

    txt.setCharacterSize(font_size);
    txt.setString(scalers::get_name(i) + ": " + (std::to_string(scalers::get(i))));
    // position so that line 0 is at top, line N below
    txt.setPosition({10.f, 10.f + (i + flags::num()) * line_height});
    txt.setPosition(txt.getPosition() + m_text_pos);
    // selected line in bold or different color
    if (i + flags::num() == cursor) {
      txt.setStyle(sf::Text::Bold);
      txt.setFillColor(sf::Color::Yellow);
    }
    m_texts.push_back(std::move(txt));
  }
}

void flag_state::draw(sf::RenderWindow &window) {
  m_text_pos = window.mapPixelToCoords({0, 0});
  for (auto &txt : m_texts) {
    window.draw(txt);
  }
}
