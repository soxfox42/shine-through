#include <pebble.h>
#include <stdint.h>
#include <stdio.h>

#include "atlas.h"

static Window *s_window;
static GBitmap *s_number_font;
static GBitmap *s_text_font;
static Layer *s_top_text;
static Layer *s_bottom_text;

const int number_gap = 12;
const int number_shift_x = 12;
const int number_shift_y = 18;

const uint8_t hours_color = 0b11011100;
const uint8_t minutes_color = 0b11110100;
const uint8_t blend_color = 0b11111100;

static void draw_number(GBitmap *fb, GPoint pos, int number, uint8_t main_color, uint8_t blend_color) {
    GRect bounds = gbitmap_get_bounds(fb);
    uint8_t *font_data = gbitmap_get_data(s_number_font);
    int font_stride = gbitmap_get_bytes_per_row(s_number_font);
    GRect font_bounds = gbitmap_get_bounds(s_number_font);
    int font_width = font_bounds.size.w / 10;
    int font_height = font_bounds.size.h;

    for (int y = 0; y < font_height; y++) {
        if (pos.y + y < 0 || pos.y + y >= bounds.size.h) {
            continue;
        }
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, pos.y + y);
        for (int x = 0; x < font_width; x++) {
            if (pos.x + x < info.min_x || pos.x + x > info.max_x) {
                continue;
            }
            int base = info.data[pos.x + x];
            int font_x = x + number * font_width;
            int index = y * font_stride + font_x / 8;
            int shift = font_x % 8;
            if ((font_data[index] >> shift) & 1) {
                if (base == GColorBlackARGB8) {
                    info.data[pos.x + x] = main_color;
                } else {
                    info.data[pos.x + x] = blend_color;
                }
            }
        }
    }
}

static void number_layer_update(Layer *layer, GContext *ctx) {
    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);

    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, 0);

    GRect font_bounds = gbitmap_get_bounds(s_number_font);
    int font_width = font_bounds.size.w / 10;
    int font_height = font_bounds.size.h;

    int two_digit_width = font_width * 2 + number_gap;
    GPoint base_point = GPoint(
        (PBL_DISPLAY_WIDTH - two_digit_width) / 2,
        (PBL_DISPLAY_HEIGHT - font_height) / 2
    );

    GPoint top_left = GPoint(base_point.x - number_shift_x / 2, base_point.y - number_shift_y / 2);
    GPoint top_right = GPoint(base_point.x - number_shift_x / 2 + font_width + number_gap, base_point.y - number_shift_y / 2);
    GPoint bottom_left = GPoint(base_point.x + number_shift_x / 2, base_point.y + number_shift_y / 2);
    GPoint bottom_right = GPoint(base_point.x + number_shift_x / 2 + font_width + number_gap, base_point.y + number_shift_y / 2);

    GBitmap *fb = graphics_capture_frame_buffer(ctx);

    draw_number(fb, top_left, now->tm_hour / 10, hours_color, 0);
    draw_number(fb, top_right, now->tm_hour % 10, hours_color, 0);
    draw_number(fb, bottom_left, now->tm_min / 10, minutes_color, blend_color);
    draw_number(fb, bottom_right, now->tm_min % 10, minutes_color, blend_color);

    graphics_release_frame_buffer(ctx, fb);
}

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
    layer_mark_dirty(window_get_root_layer(s_window));
}

static void handle_date_update(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(s_top_text);
}

static void handle_battery_update(BatteryChargeState charge) {
    layer_mark_dirty(s_bottom_text);
}

static void window_load(Window *window) {
    Layer *root = window_get_root_layer(window);
    layer_set_update_proc(root, number_layer_update);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);

    s_top_text = layer_create(GRect(4, 4, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_top_text, top_text_layer_update);
    layer_add_child(root, s_top_text);

    s_bottom_text = layer_create(GRect(4, PBL_DISPLAY_HEIGHT - 24, PBL_DISPLAY_WIDTH - 8, 20));
    layer_set_update_proc(s_bottom_text, bottom_text_layer_update);
    layer_add_child(root, s_bottom_text);

    tick_timer_service_subscribe(DAY_UNIT, handle_date_update);
    battery_state_service_subscribe(handle_battery_update);
}

static void window_unload(Window *window) {
    layer_destroy(s_top_text);
    layer_destroy(s_bottom_text);
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
