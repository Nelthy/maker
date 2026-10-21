#pragma once
#include "state_man.h"

class app {
public:
	app(std::string dir): man(dir) {}

	void start();
	template<typename T, typename...T_ARGS>
		requires std::is_base_of_v<state, T>
	void push_new_state(T_ARGS&&...args) {
		man.push_new_state<T>(args...);
	};
private:
	state_man man;
};

