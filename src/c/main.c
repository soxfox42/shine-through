#include <pebble.h>
#include <stdint.h>

static Window *s_window;
static GBitmap *s_numbers;

static void draw_number(GBitmap *fb, GPoint pos, int number, uint8_t main_color, uint8_t blend_color) {
    uint8_t *font_data = gbitmap_get_data(s_numbers);
    int font_stride = gbitmap_get_bytes_per_row(s_numbers);

    for (int y = 0; y < 80; y++) {
        GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, pos.y + y);
        for (int x = 0; x < 50; x++) {
            int base = info.data[pos.x + x];
            int index = y * font_stride + (x + number * 50) / 8;
            int shift = (x + number * 50) % 8;
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

    GBitmap *fb = graphics_capture_frame_buffer(ctx);

    draw_number(fb, GPoint(12, 36), now->tm_hour / 10, GColorPurpleARGB8, GColorPurpleARGB8);
    draw_number(fb, GPoint(72, 36), now->tm_hour % 10, GColorPurpleARGB8, GColorPurpleARGB8);
    draw_number(fb, GPoint(22, 52), now->tm_min / 10, GColorRichBrilliantLavenderARGB8, GColorShockingPinkARGB8);
    draw_number(fb, GPoint(82, 52), now->tm_min % 10, GColorRichBrilliantLavenderARGB8, GColorShockingPinkARGB8);

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
    window_set_window_handlers(s_window, (WindowHandlers){
        .load = window_load,
        .unload = window_unload,
    });
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
