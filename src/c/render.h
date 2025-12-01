#pragma once

#include <pebble.h>

void render_time(BitmapLayer *layer, GBitmap *font);
void outlines_layer_update(Layer *layer, GContext *ctx);
void set_render_outline_font(GBitmap *font);
