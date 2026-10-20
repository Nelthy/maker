#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <mutex>
#include <vector>

namespace cp6 {
struct task_data {
  char data[8 + 4 + 3];

  task_data() {};

  task_data(void *ptr, uint32_t idx = 0) {
    this->ptr() = ptr;
    this->idx() = idx;
  };

  void *&ptr() { return *reinterpret_cast<void **>(&data); }
  
  template<typename T>
  T &ptr_as() { return *(T*&)(*reinterpret_cast<void **>(&data)); }

  uint32_t &idx() { return *reinterpret_cast<uint32_t *>(data + 8); }
};

class thread_pool {
public:
  using func_t = bool (*)(task_data data, uint32_t tid);
  [[nodiscard]]
  static void *add_task(func_t func, task_data data, bool start_immediately = true);
  static void *add_task();

  static void remove_task(void *handle);

  static void start_spinning(unsigned thread_count);

  static bool is_done(const void *handle) {
    task *ptask = (task *)handle;
    return ptask->state == task_state::done;
  };

  static void restart_task(void *handle) {
    task *ptask = (task *)handle;
    assert(ptask->state == task_state::done);
    ptask->state = task_state::idle;
  };

  static void set_task_func(void *handle, func_t func) {
    task *ptask = (task *)handle;
    assert(ptask->state == task_state::done);
    ptask->func = func;
  }

  static void set_task_data(void *handle, void *data) {
    task *ptask = (task *)handle;
    assert(ptask->state == task_state::done);
    ptask->data = data;
  }

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
      while(...){
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
    task_data data;
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
