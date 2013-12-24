#ifndef progress_bar_h
#define progress_bar_h

#include <pebble.h>

typedef Layer ProgressBarLayer;

typedef enum {
	ProgressBarOrientationHorizontal = 0x00,
	ProgressBarOrientationVertical = 0x01,
} ProgressBarOrientation;

typedef struct {
	int32_t min;
	int32_t max;
	int32_t value;
	GColor frame_color;
	GColor bar_color;
	ProgressBarOrientation orientation;
} ProgressBarData;

ProgressBarLayer* progress_bar_layer_create(GRect frame);
void progress_bar_layer_destroy(ProgressBarLayer* bar);
void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max);
void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value);
void progress_bar_layer_set_frame_color(ProgressBarLayer* bar, GColor color);
void progress_bar_layer_set_bar_color(ProgressBarLayer* bar, GColor color);
void progress_bar_layer_set_orientation(ProgressBarLayer* bar, ProgressBarOrientation orientation);

#endif
