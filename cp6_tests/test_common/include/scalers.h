#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

class scalers {
private:
  struct scaler_info {
    float stride = 10;
    float value;
  };

  inline static std::unordered_map<std::string, scaler_info> m_scalers;

  inline static std::vector<std::string> m_key_list;

public:
  static void set(const std::string &name, float value) {
    if (!m_scalers.contains(name)) {
      m_key_list.push_back(name);
    }
    m_scalers[name].value = value;
  }

  static void set(int idx, float value) {
    assert(idx < m_key_list.size());
    m_scalers[m_key_list[idx]].value = value;
  }

  static void increment(const std::string &name) { set(name, get(name) - m_scalers[name].stride); }

  static void decrement(const std::string &name) {
    set(name, get(name) - m_scalers[name].stride);
  }

  static void increment(unsigned idx) {
    const std::string &name = get_name(idx);
    set(name, get(name) + m_scalers[name].stride);
  }

  static void decrement(unsigned idx) {
    const std::string &name = get_name(idx);
    set(name, get(name) - m_scalers[name].stride);
  }

  static float get(const std::string &name, float init_value = 0, float init_stride = 10) {
    if (!m_scalers.contains(name)) {
      m_key_list.push_back(name);
      m_scalers[name].value = init_value;
      m_scalers[name].stride = init_stride;
    }
    return m_scalers[name].value;
  }

  static float get(int idx) {
    assert(idx < m_key_list.size());
    return m_scalers[m_key_list[idx]].value;
  }

  static std::string get_name(int idx) {
    assert(idx < m_key_list.size());
    return m_key_list[idx];
  }

  static void clear() {
    m_key_list.clear();
    m_scalers.clear();
  }

  static void print_scalers() {
    std::cout << "====================================== \n";
    for (auto &name : m_key_list) {
      std::cout << name << ": " << std::boolalpha << m_scalers[name].value << "\n";
    }
    std::cout << "======================================";
    std::cout << std::endl;
  }

  static size_t num() { return m_key_list.size(); }
};
