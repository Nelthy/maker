#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class flags {
private:
  inline static std::unordered_map<std::string, bool> m_flags;

  inline static std::vector<std::string> m_key_list;

public:
  static void set(const std::string &name, bool value) {
    if (!m_flags.contains(name)) {
      m_key_list.push_back(name);
    }
    m_flags[name] = value;
  }

  static void set(int idx, bool value) {
    assert(idx < m_key_list.size());
    m_flags[m_key_list[idx]] = value;
  }

  static void toggle(const std::string &name) { set(name, !get(name)); }

  static void toggle(int idx) { set(idx, !get(idx)); }

  static bool get(const std::string &name, bool init_value = true) {
    if (!m_flags.contains(name)) {
      m_key_list.push_back(name);
      m_flags[name] = init_value;
    }
    return m_flags[name];
  }

  static bool get(int idx, bool init_value = true) {
    assert(idx < m_key_list.size());
    return m_flags[m_key_list[idx]];
  }

  static std::string get_name(int idx) {
    assert(idx < m_key_list.size());
    return m_key_list[idx];
  }

  static void clear() {
    m_key_list.clear();
    m_flags.clear();
  }

  static void print_flags() {
    std::cout << "====================================== \n";
    for (auto &name : m_key_list) {
      std::cout << name << ": " << std::boolalpha << m_flags[name] << "\n";
    }
    std::cout << "======================================";
    std::cout << std::endl;
  }

  static size_t num() { return m_flags.size(); }
};
