#pragma once
#include <random>

inline unsigned random_index(unsigned min, unsigned max) {
  static thread_local std::mt19937 generator(std::random_device{}());
  std::uniform_int_distribution<unsigned> distribution(min, max);
  return distribution(generator);
}
