#pragma once
#include <algorithm>
#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <string>
#include <vector>

// Performance Analyzer
class PA {
public:
  static PA &instance() {
    static PA instance; // Guaranteed to be destroyed and instantiated on first use.
    return instance;
  }

  inline void start(const char name[10]) {
    std::lock_guard<std::mutex> lock(m_map_mutex);

    m_map.emplace(name, time_data());
    time_data &t = m_map.at(name);

    t.start = std::chrono::high_resolution_clock::now();
  }

  inline void end(const char name[10]) {
    auto start = std::chrono::high_resolution_clock::now();

    std::lock_guard<std::mutex> lock(m_map_mutex);

    /*    if (!m_map.contains(name))
        {
            return;
        }*/
    time_data &t = m_map.at(name);

    t.end = std::chrono::high_resolution_clock::now();
    t.end_duration += std::chrono::high_resolution_clock::now() - start;
    t.total_duration += ((t.end - t.start));
    t.count++;
  }

  inline void print_all() {
    std::lock_guard<std::mutex> lock(m_map_mutex);

    // Sort the map entries by total duration in descending order
    std::vector<std::pair<std::string, time_data>> sorted_entries(m_map.begin(), m_map.end());
    std::sort(sorted_entries.begin(), sorted_entries.end(), [](const auto &a, const auto &b) {
      return a.second.total_duration > b.second.total_duration;
    });

    std::cout << "\n Average Times: " << "\n";
    for (const auto &entry : sorted_entries) {
      const auto &name = entry.first;
      const auto &t = entry.second;

      auto total_duration = t.total_duration - t.end_duration;
      auto avg_duration = total_duration / t.count;

      print_duration(name, avg_duration, t.count);
    }

    std::cout << "\n Total Times: " << "\n";

    for (const auto &entry : sorted_entries) {
      const auto &name = entry.first;
      const auto &t = entry.second;

      auto total_duration = t.total_duration - t.end_duration;
      print_duration(name, total_duration, 1);
    }

    std::cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
                 "++++++++++++++"
              << "\n";
    this->clear();
  }

  void clear() { m_map.clear(); }

private:
  struct time_data {
    std::chrono::high_resolution_clock::time_point start{};
    std::chrono::high_resolution_clock::time_point end{};
    std::chrono::high_resolution_clock::duration end_duration{};
    std::chrono::high_resolution_clock::duration total_duration;
    unsigned int count = 0;
  };

  std::mutex m_map_mutex;
  std::map<std::string, time_data> m_map;

  PA() = default; // Private constructor to enforce singleton pattern.
  ~PA() = default;
  PA(const PA &) = delete;
  PA &operator=(const PA &) = delete;

  void print_duration(
    const std::string &name,
    const std::chrono::high_resolution_clock::duration &duration,
    unsigned int count
  ) const {
    if (duration < std::chrono::microseconds(1)) {
      std::cout << "Task: " << name << ": "
                << std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()
                << " nanoseconds" << ", Times Ran: " << count << std::endl;
    } else if (duration < std::chrono::milliseconds(1)) {
      std::cout << "Task: " << name << ": "
                << std::chrono::duration_cast<std::chrono::microseconds>(duration).count()
                << " microseconds" << ", Times Ran: " << count << std::endl;
    } else if (duration < std::chrono::seconds(1)) {
      std::cout << "Task: " << name << ": "
                << std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()
                << " milliseconds" << ", Times Ran: " << count << std::endl;
    } else {
      std::cout << "Task: " << name << ": "
                << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << " seconds"
                << ", Times Ran: " << count << std::endl;
    }
  }
};
