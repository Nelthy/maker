#include "app.h"
#include "avbd_test_state.h"
#include "flag_state.h"
#include "flags.h"
#include "scalers.h"
#include "thread_pool.h"

int main() {
  flags::get(
    "general controls: R to reset/update the scene, arrow keys to move camera, +- keys to zoom in and out"
  );
  flags::get(
    "spawning controls: E to spawn shape (shape_type), D to delete shape, Z to delete random polygon"
  );
  flags::get(
    "menu controls: P to go up, N to go down, K to increment, Shift + k to decrement"
  );


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
