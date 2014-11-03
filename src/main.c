#include <pebble.h>
#include "skipstone.h"

int main(void) {
	skipstone_init();
	app_event_loop();
	skipstone_deinit();
}
