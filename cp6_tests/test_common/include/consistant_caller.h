#pragma once

class consistant_caller {
public:
  template<typename T, typename... TArgs>
  unsigned update(float dt, const float h, T func, TArgs &&...args) {
    unsigned current_step_num = 0;
    elapsed += dt;
    while (h < elapsed) {
      func(args...);
      current_step_num++;
      elapsed -= h;
    }
    return current_step_num;
  }

private:
  float elapsed = 0;
};

/*
Example use:

your_state::update(){
  static consistant_caller updater;
  //prints hello every second
  updater.update(dt, 1, [&](std::string s){std::cout << s << std::endl;}, "hello");} }
*/
