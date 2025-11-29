#include <pebble.h>
#include <stdint.h>
#include <stdio.h>

#include "atlas.h"
#include "render.h"

static GBitmap *s_number_font;
static GBitmap *s_text_font;

static Window *s_window;
static GBitmap *s_time_bitmap;
static BitmapLayer *s_time;
static Layer *s_top_text;
static Layer *s_bottom_text;

static GRect get_atlas(char c) {
    if (c >= 'a' && c <= 'z') {
        c += 'A' - 'a';
    }
    for (unsigned int i = 0; i < ARRAY_LENGTH(TEXT_FONT_ATLAS); i++) {
        if (TEXT_FONT_ATLAS[i].character == c) {
            return TEXT_FONT_ATLAS[i].rect;
        }
    }
    return GRectZero;
}

static void draw_text(Layer *layer, GContext *ctx, char *text, bool right) {
    GRect bounds = layer_get_bounds(layer);
    int x = 0;
    size_t len = strlen(text);

    for (size_t i = 0; i < len; i++) {
        size_t j = right ? len - i - 1 : i;
        if (text[j] == ' ') {
            x += 8;
            continue;
        }

        GRect src_rect = get_atlas(text[j]);
        gbitmap_set_bounds(s_text_font, src_rect);
        GRect dest_rect = right
            ? GRect(bounds.size.w - x - src_rect.size.w, 0, src_rect.size.w, src_rect.size.h)
            : GRect(x, 0, src_rect.size.w, src_rect.size.h);
        graphics_draw_bitmap_in_rect(ctx, s_text_font, dest_rect);

        x += src_rect.size.w + 2;
    }
}

static void top_text_layer_update(Layer *layer, GContext *ctx) {
    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);
    char buf[16];
    strftime(buf, sizeof(buf), "%a %d", now);
    draw_text(layer, ctx, buf, false);
}

static void bottom_text_layer_update(Layer *layer, GContext *ctx) {
    int battery_level = battery_state_service_peek().charge_percent;
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", battery_level);
    draw_text(layer, ctx, buf, true);
}

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    render_time(s_time, s_number_font);

    if (units_changed & DAY_UNIT) {
        layer_mark_dirty(s_top_text);
    }
}

static void handle_battery_update(BatteryChargeState charge) {
    layer_mark_dirty(s_bottom_text);
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);

    s_top_text = layer_create(GRect(4, 4, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_top_text, top_text_layer_update);
    layer_add_child(root, s_top_text);

    s_bottom_text = layer_create(GRect(4, PBL_DISPLAY_HEIGHT - 24, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_bottom_text, bottom_text_layer_update);
    layer_add_child(root, s_bottom_text);

    s_time_bitmap = gbitmap_create_blank(GSize(144, 108), GBitmapFormat2BitPalette);
    static GColor palette[4];
    palette[0] = GColorBlack;
    palette[1] = GColorOrange;
    palette[2] = GColorBrightGreen;
    palette[3] = GColorYellow;
    gbitmap_set_palette(s_time_bitmap, palette, false);

    s_time = bitmap_layer_create(GRect(0, 30, 144, 108));
    bitmap_layer_set_bitmap(s_time, s_time_bitmap);
    layer_add_child(root, bitmap_layer_get_layer(s_time));

    tick_timer_service_subscribe(MINUTE_UNIT | DAY_UNIT, handle_tick);
    handle_tick(NULL, MINUTE_UNIT);
    battery_state_service_subscribe(handle_battery_update);
}

static void window_unload(Window *window) {
    layer_destroy(s_top_text);
    layer_destroy(s_bottom_text);
    bitmap_layer_destroy(s_time);
    gbitmap_destroy(s_time_bitmap);
    tick_timer_service_unsubscribe();
}

static void app_init(void) {
    s_number_font = gbitmap_create_with_resource(RESOURCE_ID_NUMBER_FONT);
    s_text_font = gbitmap_create_with_resource(RESOURCE_ID_TEXT_FONT);

    s_window = window_create();
    window_set_window_handlers(
        s_window,
        (WindowHandlers){
            .load = window_load,
            .unload = window_unload,
        }
    );
    window_set_background_color(s_window, GColorBlack);
    window_stack_push(s_window, true);
}

static void app_deinit(void) {
    window_destroy(s_window);
    gbitmap_destroy(s_number_font);
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
