#pragma once
#include <vector>
#include <atomic>
#include <cassert>
#include <cstdint>

namespace cp6 {
class thread_pool {
public:
  using func_t = bool (*)(void *data);
  [[nodiscard]]
  static void *add_task(func_t func, void *data);

  static void remove_task(void *handle);

  static void start_spinning(unsigned thread_count);

  static bool is_done(const void *handle) {
    static uint32_t current_loop = 0;
    task *ptask = (task *)handle;

    return ptask->state == task_state::done;
  };

  static void stop_spinning();

  static thread_pool &inst() {
    static thread_pool instance{};
    return instance;
  }

  static bool has_handle(const void *handle);

  static uint32_t task_pool_size();

  static uint32_t get_created_task_num() { return inst().created_task_num; }

  static uint32_t get_current_state(void *handle) {
    task *rtask = (task *)handle;
    return (uint32_t)((task *)handle)->state.load();
  };

  /*
    usage:
      task* help_handle = nullptr;
      while(!thread_pool::is_done(some_handle)){
        help_handle = thread_pool::main_help(help_handle);
      }

  */
  [[nodiscard("returns the next handle in the task list")]]
  static void *main_help(void *start_task = nullptr);

private:
  static void thread_spin(uint32_t tid);

  thread_pool() = default;

  ~thread_pool();

  enum class task_state : uint8_t { null, idle, claimed, done, in_task_pool };

  struct task {
    func_t func = nullptr;
    void *data = nullptr;
    std::atomic<task_state> state = task_state::null;
    std::atomic<task *> next = nullptr;
    std::atomic<task *> last = nullptr;
  };

  struct thread_state {
    std::atomic<bool> is_spinning = false;
  };

  std::vector<thread_state *> m_thread_states;
  std::atomic<task *> first = nullptr;
  std::atomic<task *> task_pool_first = nullptr;
  std::atomic<bool> is_spinning;
  std::mutex list_mtx;

  uint32_t created_task_num = 0;
};
} // namespace cp6
