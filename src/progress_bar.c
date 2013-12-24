#include "progress_bar.h"

static void progress_bar_layer_update(ProgressBarLayer *bar, GContext *ctx);

ProgressBarLayer* progress_bar_layer_create(GRect frame) {
	ProgressBarLayer *bar = layer_create_with_data(frame, sizeof(ProgressBarData));
	layer_set_update_proc(bar, progress_bar_layer_update);
	layer_mark_dirty(bar);

	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->min = 0;
	data->max = 100;
	data->value = 0;
	data->frame_color = GColorBlack;
	data->bar_color = GColorBlack;
	data->orientation = ProgressBarOrientationHorizontal;

	return bar;
}

void progress_bar_layer_destroy(ProgressBarLayer* bar) {
	layer_destroy(bar);
}

void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->min = min;
	data->max = max;
	layer_mark_dirty(bar);
}

void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->value = value;
	layer_mark_dirty(bar);
}

void progress_bar_layer_set_frame_color(ProgressBarLayer* bar, GColor color) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->frame_color = color;
	layer_mark_dirty(bar);
}

void progress_bar_layer_set_bar_color(ProgressBarLayer* bar, GColor color) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->bar_color = color;
	layer_mark_dirty(bar);
}

void progress_bar_layer_set_orientation(ProgressBarLayer* bar, ProgressBarOrientation orientation) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	data->orientation = orientation;
	layer_mark_dirty(bar);
}

static void progress_bar_layer_update(ProgressBarLayer *bar, GContext *ctx) {
	ProgressBarData *data = (ProgressBarData *)layer_get_data(bar);
	GRect bounds = layer_get_bounds(bar);

	graphics_context_set_stroke_color(ctx, data->frame_color);
	graphics_draw_round_rect(ctx, bounds, 4);

	if (data->orientation == ProgressBarOrientationHorizontal) {
		bounds.size.w = ((data->value - data->min) * bounds.size.w) / (data->max - data->min);
	} else if (data->orientation == ProgressBarOrientationVertical) {
		bounds.origin.y = bounds.size.h - (((data->value - data->min) * bounds.size.h) / (data->max - data->min));
		bounds.size.h = ((data->value - data->min) * bounds.size.h) / (data->max - data->min);
	}

	graphics_context_set_fill_color(ctx, data->bar_color);
	graphics_fill_rect(ctx, bounds, 4, GCornersAll);
}
