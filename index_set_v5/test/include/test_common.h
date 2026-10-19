#pragma once
#include <iostream>

struct indent {
  static inline int level = 0;
  static constexpr int width = 2;

  friend std::ostream &operator<<(std::ostream &os, const indent &) {
    return os << std::string(level * width, ' ');
  }
};

struct scoped_indent {
  scoped_indent() { ++indent::level; }

  ~scoped_indent() { --indent::level; }
};

#define VERBOSE

#ifdef VERBOSE
#define LOG(name) std::cout << indent() << #name << ": " << name << std::endl
#define LOG_FUNC_NAME()                                                                            \
  scoped_indent _s;                                                                                \
  std::cout << indent() << __FUNCTION__ << std::endl

#endif

#ifndef VERBOSE
#define LOG(name)
#define LOG_FUNC_NAME()
#endif

#define test_assert(e)                                                                             \
  if (!(e)) {                                                                                      \
    scoped_indent _s;                                                                              \
    std::cout << indent() << "Test assertion failed: " #e << ", file " << __FILE__ << ", "    \
              << "line " << __LINE__ << "." << std::endl;                                          \
  }
