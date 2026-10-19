#pragma once
#include "maths.h"
#include <SFML/System/Vector2.hpp>

inline sf::Vector2f &to_sf(vec2f &v) { return *reinterpret_cast<sf::Vector2f *>(&v); }

inline const sf::Vector2f &to_sf(const vec2f &v) {
  return *reinterpret_cast<const sf::Vector2f *>(&v);
}

inline vec2f &from_sf(sf::Vector2f &v) { return *reinterpret_cast<vec2f *>(&v); }

inline const vec2f &from_sf(const sf::Vector2f &v) { return *reinterpret_cast<const vec2f *>(&v); }
