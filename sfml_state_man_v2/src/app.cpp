#include "app.h"
void app::start() {
	while(!man.is_quit()) {
		man.update();
	}
}
