#include "thread_pool.h"
#include <thread>

namespace cp6 {
void *thread_pool::add_task(
  func_t func,
  task_data data,
  bool start_immediately
) {
  inst().list_mtx.lock();

  task *new_task = nullptr;
  if (inst().task_pool_first != nullptr) {
    new_task = inst().task_pool_first.load();
    inst().task_pool_first.store(new_task->next);

    new_task->func = func;
    new_task->data = data;
    new_task->state = thread_pool::task_state::claimed;
    new_task->next = nullptr;
    new_task->last = nullptr;
  }

  if (new_task == nullptr) {
    inst().created_task_num++;
    new_task = new task{func, data};
    new_task->state = task_state::claimed;
  }

  task *old_first = inst().first.exchange(new_task);
  if (old_first != nullptr) { old_first->last = new_task; }
  new_task->next = old_first;

  if (start_immediately) {
    new_task->state.store(task_state::idle);
  } else {
    new_task->state.store(task_state::done);
  }

  inst().list_mtx.unlock();
  return new_task;
}

void *thread_pool::add_task() {
  inst().list_mtx.lock();

  task *new_task = nullptr;
  if (inst().task_pool_first != nullptr) {
    new_task = inst().task_pool_first.load();
    inst().task_pool_first.store(new_task->next);

    new_task->func = nullptr;
    new_task->data = nullptr;
    new_task->state = thread_pool::task_state::claimed;
    new_task->next = nullptr;
    new_task->last = nullptr;
  }

  if (new_task == nullptr) {
    inst().created_task_num++;
    new_task = new task{nullptr, nullptr};
    new_task->state = task_state::claimed;
  }

  task *old_first = inst().first.exchange(new_task);
  if (old_first != nullptr) { old_first->last = new_task; }
  new_task->next = old_first;

  new_task->state.store(task_state::done);

  inst().list_mtx.unlock();
  return new_task;
}

void thread_pool::remove_task(void *handle) {
  assert(inst().is_done(handle));
  inst().list_mtx.lock();
  task *rtask = (task *)handle;
  task *last = rtask->last;
  task *next = rtask->next;

  if (last != nullptr) {
    last->next.store(rtask->next);
  } else {
    // rtask is first
    inst().first.store(rtask->next);
  }

  if (next != nullptr) {
    next->last.store(rtask->last);
  } else {
    // rtask is last
  }

  task *old_first = inst().task_pool_first.exchange(rtask);
  rtask->next = old_first;
  rtask->state = task_state::in_task_pool;

  inst().list_mtx.unlock();
}

void thread_pool::start_spinning(unsigned thread_count) {
  assert(!inst().is_spinning);
  inst().is_spinning = true;

  for (uint32_t t = 0; t < thread_count; t++) {
    inst().m_thread_states.push_back(new thread_state{});
  }
  for (uint32_t t = 0; t < thread_count; t++) {
    std::thread thread{thread_spin, t};
    thread.detach();
  }
};

void thread_pool::stop_spinning() {
  if (inst().is_spinning) {
    inst().is_spinning = false;
    // wait for all the threads to start spinning

    for (uint32_t t = 0; t < inst().m_thread_states.size(); t++) {
      while (inst().m_thread_states[t]->is_spinning.load() == false) {};
    }

    // tell all threads to stop spinning
    for (uint32_t t = 0; t < inst().m_thread_states.size(); t++) {
      inst().m_thread_states[t]->is_spinning.store(false);
    }

    // wait for all the threads to stop spinning
    for (uint32_t t = 0; t < inst().m_thread_states.size(); t++) {
      while (inst().m_thread_states[t]->is_spinning.load() == false) {};
    }

    // delete the thread states
    for (uint32_t t = 0; t < inst().m_thread_states.size(); t++) {
      delete inst().m_thread_states[t];
    }
    inst().m_thread_states.resize(0);
  }
};

bool thread_pool::has_handle(const void *handle) {
  uint32_t loop_num = 0;
  task *curr = inst().first;
  while (true) {
    assert(curr->state.load() != task_state::in_task_pool);

    if (curr == handle) { return true; }

    if (curr == nullptr) { return false; }

    curr = curr->next;
    loop_num++;
  }
}

uint32_t thread_pool::task_pool_size() {
  uint32_t num = 0;
  task *curr = inst().task_pool_first;
  while (curr != nullptr) {
    num++;
    curr = curr->next;
  }
  return num;
}

[[nodiscard("returns the next handle in the task list")]]
void *thread_pool::main_help(void *start_task) {
  task *curr_task = (task *)start_task;
  if (curr_task == nullptr) {
    curr_task = inst().first;
    return curr_task;
  }

  task_state expected = task_state::idle;
  if (curr_task->state.compare_exchange_strong(expected, task_state::claimed)) {
    assert(curr_task->state.load() == task_state::claimed);

    if (curr_task->func(curr_task->data, 0)) {
      curr_task->state.store(task_state::done);
    } else {
      curr_task->state.store(task_state::idle);
    }
  }

  if (expected == task_state::in_task_pool) {
    curr_task = inst().first;
    return curr_task;
  }

  return curr_task->next;
}

void thread_pool::thread_spin(uint32_t tid) {
  thread_state *state = inst().m_thread_states[tid];
  state->is_spinning.store(true);

  task *curr_task = nullptr;
  uint32_t claim_dist = 0;
  while (state->is_spinning) {
    if (curr_task == nullptr) {
      curr_task = inst().first;
      continue;
    }

    task_state expected = task_state::idle;
    if (
      curr_task->state.compare_exchange_strong(expected, task_state::claimed)
    ) {
      assert(curr_task->state.load() == task_state::claimed);

      if (curr_task->func(curr_task->data, tid + 1)) {
        curr_task->state.store(task_state::done);
      } else {
        curr_task->state.store(task_state::idle);
      }
    }

    if (expected == task_state::in_task_pool) {
      curr_task = inst().first;
      continue;
    }

    curr_task = curr_task->next;
  }

  state->is_spinning = true;
}

thread_pool::~thread_pool() {
  uint32_t deleted_task_num = 0;
  stop_spinning();
  // delete all tasks in task pool
  {
    task *rtask = inst().task_pool_first;
    while (rtask != nullptr) {
      task *next = rtask->next;
      delete rtask;
      rtask = next;
      deleted_task_num++;
    }
  }
  // delete all unfinished tasks
  {
    task *rtask = inst().first;
    while (rtask != nullptr) {
      task *next = rtask->next;
      delete rtask;
      rtask = next;
      deleted_task_num++;
    }
  }
  assert(deleted_task_num == inst().created_task_num && "memory leak!");
};

} // namespace cp6
