#include "app.h"
#include "avbd_test_state.h"
#include "flag_state.h"
#include "scalers.h"
#include "thread_pool.h"

int main() {
  scalers::get("fps");
  scalers::get("collider number");
  scalers::get("contact number");
  app app("cp6_tests");
  app.push_new_state<flag_state>();
  app.push_new_state<avbd_test_state>();
  // what works best emperically
  cp6::thread_pool::start_spinning(std::thread::hardware_concurrency() / 2);
  app.start();
  cp6::thread_pool::stop_spinning();
}
