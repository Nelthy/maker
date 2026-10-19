#include "app.h"
#include "collision_test_state.h"
#include "flag_state.h"
#include "scalers.h"

int main() {
  scalers::get("fps");
  scalers::get("collider number");
  scalers::get("contact number");
  app app("cp6_tests");
  app.push_new_state<flag_state>();
  app.push_new_state<collision_test_state>();
  app.start();
}
