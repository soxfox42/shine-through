#include <pebble.h>
#include <stdint.h>

static Window *s_window;
static GBitmap *s_numbers;

const int number_gap = 12;
const int number_shift_x = 12;
const int number_shift_y = 18;

const uint8_t hours_color = 0b11110100;
const uint8_t minutes_color = 0b11001011;
const uint8_t blend_color = 0b11110111;

static void draw_number(GBitmap *fb, GPoint pos, int number, uint8_t main_color, uint8_t blend_color) {
    GRect bounds = gbitmap_get_bounds(fb);
    uint8_t *font_data = gbitmap_get_data(s_numbers);
    int font_stride = gbitmap_get_bytes_per_row(s_numbers);
    GRect font_bounds = gbitmap_get_bounds(s_numbers);
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

static void layer_update(Layer *layer, GContext *ctx) {
    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);

    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, bounds, 0, 0);

    GRect font_bounds = gbitmap_get_bounds(s_numbers);
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

static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(window_get_root_layer(s_window));
}

static void window_load(Window *window) {
    Layer *layer = window_get_root_layer(window);
    layer_set_update_proc(layer, layer_update);
    tick_timer_service_subscribe(MINUTE_UNIT, handle_tick);
}

static void window_unload(Window *window) {
    tick_timer_service_unsubscribe();
}

static void app_init(void) {
    s_numbers = gbitmap_create_with_resource(RESOURCE_ID_NUMBERS);

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
    gbitmap_destroy(s_numbers);
}

int main(void) {
    app_init();
    app_event_loop();
    app_deinit();
}
