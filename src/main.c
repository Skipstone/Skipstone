#include <pebble.h>
#include "appmessage.h"
#include "windows/playerlist.h"

static void init(void) {
	appmessage_init();
	playerlist_init();
}

static void deinit(void) {
	playerlist_destroy();
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
